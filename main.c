#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#define MAX_PRODUCTS 50
#define NAME_LEN 64

typedef struct {
    char name[NAME_LEN];
    double price;
    int quantity;
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


static void list_products(const Product *arr, int count) {
    printf("\n--- SEZNAM PRODUKTU ---\n");
    printf("%-5s | %-20s | %-10s | %-8s\n", "ID", "Nazev", "Cena", "Ks");
    printf("------------------------------------------------------\n");
    if (count == 0) {
        printf("Sklad je prazdny.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("[%2d]  | %-20s | %10.2f | %8d", 
               i, arr[i].name, arr[i].price, arr[i].quantity);
        if (arr[i].quantity < 5) printf(" [DOCHAZI!]");
        printf("\n");
    }
}

static void search_by_name(const Product *arr, int count) {
    char name[NAME_LEN];
    read_line("Zadejte presny nazev produktu", name, sizeof(name));
    
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i].name, name) == 0) {
            printf("\nNalezeno: ID [%d] %s, Cena: %.2f, Skladem: %d ks\n",
                   i, arr[i].name, arr[i].price, arr[i].quantity);
            found = 1;
        }
    }
    if (!found) printf("\nProdukt s nazvem '%s' nebyl nalezen.\n", name);
}

static void search_by_price(const Product *arr, int count) {
    double min_p, max_p;
    if (!read_double("Minimalni cena", &min_p)) return;
    if (!read_double("Maximalni cena: ", &max_p)) return;

    printf("\nProdukty v rozmezi %.2f - %.2f:\n", min_p, max_p);
    int found = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i].price >= min_p && arr[i].price <= max_p) {
            printf("[%d] %s (%.2f Kc)\n", i, arr[i].name, arr[i].price);
            found = 1;
        }
    }
    if (!found) printf("Zadne produkty neodpovidaji cenovemu rozpeti\n");
}

static void show_detail(const Product *arr, int count) {
    int idx;
    if (!read_int("Zadej ID produktu pro detail ", &idx) || idx < 0 || idx >= count) {
        printf("Neplatne ID.\n");
        return;
    }
    printf("\n--- DETAIL PRODUKTU ---\n");
    printf("Nazev:   %s\n", arr[idx].name);
    printf("Cena:    %.2f Kc\n", arr[idx].price);
    printf("Skladem: %d ks\n", arr[idx].quantity);
    printf("Celkova hodnota zasob: %.2f Kc\n", arr[idx].price * arr[idx].quantity);
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
    if (!read_int("Zadejte Id produktu k uprave: ", &idx) || idx < 0 || idx >= count) {
        printf("Neplatne ID.\n");
        return;
    }
    
    printf("Upravujete: %s (Enter pro zachovani hodnoty\n", arr[idx].name);
    
    char buf[128];
    if (read_line("Novy nazev: ", buf, sizeof(buf)) && buf[0] != '\0') {
        strncpy(arr[idx].name, buf, NAME_LEN - 1);
    }
    
    if (read_line("Nova cena:", buf, sizeof(buf)) && buf[0] != '\0') {
        parse_double(buf, &arr[idx].price);
    }

    if (read_line("Nove mnozstvi:", buf, sizeof(buf)) && buf[0] != '\0') {
        parse_int(buf, &arr[idx].quantity);
    }
    printf("Produkt byl aktualizovan.\n");
}

static void add_product(Product *arr, int *count) {
    if (*count >= MAX_PRODUCTS) {
        printf("Sklad je plny.\n");
        return;
    }
    Product p;
    if (!read_line("Nazev produktu:", p.name, NAME_LEN)) return;
    if (!read_double("Cena :", &p.price)) return;
    if (!read_int("Mnozstvi: ", &p.quantity)) return;
    
    arr[*count] = p;
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
    Product products[MAX_PRODUCTS];
    int count = 0;

    strcpy(products[count].name, "Kocici granule\"");
    products[count].price = 220.0;
    products[count].quantity = 18;
    count++;

    char choice_str[10];
    while (1) {
        printMenu();
        if (!read_line("", choice_str, sizeof(choice_str))) break;
        
        char choice = (char)toupper((unsigned char)choice_str[0]);
        if (choice == 'X') break;

        switch (choice) {
            case '1': list_products(products, count); break;
            case '2': search_by_name(products, count); break;
            case '3': search_by_price(products, count); break;
            case '4': show_detail(products, count); break;
            case '5': delete_product(products, &count); break;
            case '6': edit_product(products, count); break;
            case '7': add_product(products, &count); break;
            default: printf("Neplatna volba.\n");
        }
    }

    printf("Aplikace ukoncena.\n");
    return 0;
}