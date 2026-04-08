#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NAME_LEN 64
#define EAN_LEN 32
#define INITIAL_CAPACITY 2

typedef struct {
    char name[NAME_LEN];
    char ean[EAN_LEN];
    double buy_price;
    double sale_price_no_dph;
    int quantity;
    int dph_level;   // např. 12 nebo 21
} Product;

static void trim_newline(char *s) {
    if (!s) return;
    size_t n = strlen(s);
    while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
        s[n - 1] = '\0';
        n--;
    }
}

static int read_line(const char *prompt, char *out, size_t out_size) {
    if (prompt && prompt[0] != '\0') {
        printf("%s", prompt);
        fflush(stdout);
    }
    if (!fgets(out, (int)out_size, stdin)) return 0;
    trim_newline(out);
    return 1;
}

static int parse_int(const char *s, int *out) {
    if (!s || *s == '\0') return 0;
    char *end;
    long v = strtol(s, &end, 10);
    if (*end != '\0') return 0;
    *out = (int)v;
    return 1;
}

static int parse_double(const char *s, double *out) {
    if (!s || *s == '\0') return 0;
    char *end;
    double v = strtod(s, &end);
    if (*end != '\0') return 0;
    *out = v;
    return 1;
}

static int read_int(const char *prompt, int *out) {
    char buf[128];
    if (!read_line(prompt, buf, sizeof(buf))) return 0;
    return parse_int(buf, out);
}

static int read_double(const char *prompt, double *out) {
    char buf[128];
    if (!read_line(prompt, buf, sizeof(buf))) return 0;
    return parse_double(buf, out);
}

static double price_with_dph(double price, int dph_level) {
    return price * (1.0 + dph_level / 100.0);
}

static int ensure_capacity(Product **arr, int count, int *capacity) {
    if (count < *capacity) return 1;

    int new_capacity = (*capacity == 0) ? INITIAL_CAPACITY : (*capacity * 2);
    Product *tmp = realloc(*arr, new_capacity * sizeof(Product));
    if (!tmp) {
        printf("Nepodarilo se rozsirit pamet.\n");
        return 0;
    }

    *arr = tmp;
    *capacity = new_capacity;
    return 1;
}

static void list_products(const Product *arr, int count) {
    printf("\n--- SEZNAM PRODUKTU ---\n");
    printf("%-5s | %-20s | %-15s | %-10s | %-10s | %-6s | %-5s\n",
           "ID", "Nazev", "EAN", "Bez DPH", "S DPH", "Ks", "DPH");
    printf("-------------------------------------------------------------------------------\n");

    if (count == 0) {
        printf("Sklad je prazdny.\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        printf("[%2d]  | %-20s | %-15s | %10.2f | %10.2f | %6d | %3d%%",
               i,
               arr[i].name,
               arr[i].ean,
               arr[i].sale_price_no_dph,
               price_with_dph(arr[i].sale_price_no_dph, arr[i].dph_level),
               arr[i].quantity,
               arr[i].dph_level);

        if (arr[i].quantity < 5) printf(" [DOCHAZI!]");
        printf("\n");
    }
}

static void search_by_name(const Product *arr, int count) {
    char name[NAME_LEN];
    if (!read_line("Zadejte presny nazev produktu: ", name, sizeof(name))) return;

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i].name, name) == 0) {
            printf("\nNalezeno: ID [%d] %s | EAN: %s | Bez DPH: %.2f | S DPH: %.2f | Skladem: %d ks\n",
                   i,
                   arr[i].name,
                   arr[i].ean,
                   arr[i].sale_price_no_dph,
                   price_with_dph(arr[i].sale_price_no_dph, arr[i].dph_level),
                   arr[i].quantity);
            found = 1;
        }
    }

    if (!found) {
        printf("\nProdukt s nazvem '%s' nebyl nalezen.\n", name);
    }
}

static void search_by_price(const Product *arr, int count) {
    double min_p, max_p;
    if (!read_double("Minimalni prodejni cena bez DPH: ", &min_p)) return;
    if (!read_double("Maximalni prodejni cena bez DPH: ", &max_p)) return;

    printf("\nProdukty v rozmezi %.2f - %.2f (bez DPH):\n", min_p, max_p);

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i].sale_price_no_dph >= min_p && arr[i].sale_price_no_dph <= max_p) {
            printf("[%d] %s | Bez DPH: %.2f | S DPH: %.2f\n",
                   i,
                   arr[i].name,
                   arr[i].sale_price_no_dph,
                   price_with_dph(arr[i].sale_price_no_dph, arr[i].dph_level));
            found = 1;
        }
    }

    if (!found) {
        printf("Zadne produkty neodpovidaji cenovemu rozpeti.\n");
    }
}

static void show_detail(const Product *arr, int count) {
    int idx;
    if (!read_int("Zadej ID produktu pro detail: ", &idx) || idx < 0 || idx >= count) {
        printf("Neplatne ID.\n");
        return;
    }

    printf("\n--- DETAIL PRODUKTU ---\n");
    printf("Nazev:                 %s\n", arr[idx].name);
    printf("EAN:                   %s\n", arr[idx].ean);
    printf("Cena nakupu:           %.2f Kc\n", arr[idx].buy_price);
    printf("Prodejni cena bez DPH: %.2f Kc\n", arr[idx].sale_price_no_dph);
    printf("Prodejni cena s DPH:   %.2f Kc\n", price_with_dph(arr[idx].sale_price_no_dph, arr[idx].dph_level));
    printf("DPH:                   %d %%\n", arr[idx].dph_level);
    printf("Skladem:               %d ks\n", arr[idx].quantity);
    printf("Hodnota zasob (nakup): %.2f Kc\n", arr[idx].buy_price * arr[idx].quantity);
}

static void delete_product(Product *arr, int *count) {
    int idx;
    if (!read_int("Zadejte ID produktu k odstraneni: ", &idx) || idx < 0 || idx >= *count) {
        printf("Neplatne ID.\n");
        return;
    }

    for (int i = idx; i < *count - 1; i++) {
        arr[i] = arr[i + 1];
    }

    (*count)--;
    printf("Produkt byl odstranen.\n");
}

static void edit_product(Product *arr, int count) {
    int idx;
    char buf[128];

    if (!read_int("Zadejte ID produktu k uprave: ", &idx) || idx < 0 || idx >= count) {
        printf("Neplatne ID.\n");
        return;
    }

    printf("Upravujete: %s (Enter pro zachovani hodnoty)\n", arr[idx].name);

    if (read_line("Novy nazev: ", buf, sizeof(buf)) && buf[0] != '\0') {
        strncpy(arr[idx].name, buf, NAME_LEN - 1);
        arr[idx].name[NAME_LEN - 1] = '\0';
    }

    if (read_line("Nove EAN: ", buf, sizeof(buf)) && buf[0] != '\0') {
        strncpy(arr[idx].ean, buf, EAN_LEN - 1);
        arr[idx].ean[EAN_LEN - 1] = '\0';
    }

    if (read_line("Nova cena nakupu: ", buf, sizeof(buf)) && buf[0] != '\0') {
        parse_double(buf, &arr[idx].buy_price);
    }

    if (read_line("Nova prodejni cena bez DPH: ", buf, sizeof(buf)) && buf[0] != '\0') {
        parse_double(buf, &arr[idx].sale_price_no_dph);
    }

    if (read_line("Nove mnozstvi: ", buf, sizeof(buf)) && buf[0] != '\0') {
        parse_int(buf, &arr[idx].quantity);
    }

    if (read_line("Nove DPH (napr. 12 nebo 21): ", buf, sizeof(buf)) && buf[0] != '\0') {
        parse_int(buf, &arr[idx].dph_level);
    }

    printf("Produkt byl aktualizovan.\n");
}

static void add_product(Product **arr, int *count, int *capacity) {
    if (!ensure_capacity(arr, *count, capacity)) {
        return;
    }

    Product p;

    if (!read_line("Nazev produktu: ", p.name, NAME_LEN)) return;
    if (!read_line("EAN: ", p.ean, EAN_LEN)) return;
    if (!read_double("Cena nakupu: ", &p.buy_price)) return;
    if (!read_double("Prodejni cena bez DPH: ", &p.sale_price_no_dph)) return;
    if (!read_int("Mnozstvi: ", &p.quantity)) return;
    if (!read_int("DPH level (napr. 12 nebo 21): ", &p.dph_level)) return;

    (*arr)[*count] = p;
    (*count)++;

    printf("Produkt pridan.\n");
}

void printMenu(void) {
    printf("\n================= MENU SKLADU =================\n");
    printf("1) Vypis vsech produktu\n");
    printf("2) Vyhledat podle nazvu\n");
    printf("3) Vyhledat podle ceny\n");
    printf("4) Detail produktu (podle ID)\n");
    printf("5) Odstranit produkt (podle ID)\n");
    printf("6) Upravit produkt\n");
    printf("7) Pridat novy produkt\n");
    printf("X) Ukoncit program\n");
    printf("===============================================\n");
    printf("Vase volba: ");
}

int main(void) {
    int count = 0;
    int capacity = INITIAL_CAPACITY;
    Product *products = malloc(capacity * sizeof(Product));

    if (!products) {
        printf("Nepodarilo se alokovat pamet.\n");
        return 1;
    }

    strcpy(products[count].name, "Kocici granule");
    strcpy(products[count].ean, "8591234567890");
    products[count].buy_price = 180.0;
    products[count].sale_price_no_dph = 220.0;
    products[count].quantity = 18;
    products[count].dph_level = 21;
    count++;

    char choice_str[10];

    while (1) {
        printMenu();

        if (!read_line("", choice_str, sizeof(choice_str))) break;

        char choice = (char)toupper((unsigned char)choice_str[0]);
        if (choice == 'X') break;

        switch (choice) {
            case '1':
                list_products(products, count);
                break;
            case '2':
                search_by_name(products, count);
                break;
            case '3':
                search_by_price(products, count);
                break;
            case '4':
                show_detail(products, count);
                break;
            case '5':
                delete_product(products, &count);
                break;
            case '6':
                edit_product(products, count);
                break;
            case '7':
                add_product(&products, &count, &capacity);
                break;
            default:
                printf("Neplatna volba.\n");
        }
    }

    free(products);
    printf("Aplikace ukoncena.\n");
    return 0;
}