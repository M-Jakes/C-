#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

void printMenu(void);

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
    if (!s) return 0;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 0;

    errno = 0;
    char *end = NULL;
    long v = strtol(s, &end, 10);
    if (errno != 0) return 0;
    while (end && isspace((unsigned char)*end)) end++;
    if (!end || *end != '\0') return 0;
    if (v < -2147483648L || v > 2147483647L) return 0;

    *out = (int)v;
    return 1;
}

static int parse_double(const char *s, double *out) {
    if (!s) return 0;
    while (isspace((unsigned char)*s)) s++;
    if (*s == '\0') return 0;

    errno = 0;
    char *end = NULL;
    double v = strtod(s, &end);
    if (errno != 0) return 0;
    while (end && isspace((unsigned char)*end)) end++;
    if (!end || *end != '\0') return 0;

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

static double product_value(const Product *p) {
    return p->price * (double)p->quantity;
}

/* SIMPLE LIST OUTPUT (NOT A TABLE) */
static void list_products(const Product *arr, int count) {
    printf("\nVYPIS PRODUKTU (pocet: %d / %d)\n", count, MAX_PRODUCTS);
    if (count == 0) {
        printf("Sklad je prazdny.\n");
        return;
    }
    for (int i = 0; i < count; i++) {
        printf("[%d] %s | cena: %.2f | kusy: %d",
               i, arr[i].name, arr[i].price, arr[i].quantity);
        if (arr[i].quantity < 5) printf("  LOW");
        printf("\n");
    }
}

static void show_product_detail(const Product *arr, int count) {
    if (count == 0) {
        printf("\nSklad je prazdny.\n");
        return;
    }
    int idx;
    if (!read_int("Zadej index produktu: ", &idx)) {
        printf("Spatny vstup.\n");
        return;
    }
    if (idx < 0 || idx >= count) {
        printf("Index mimo rozsah.\n");
        return;
    }

    printf("\nDETAIL PRODUKTU\n");
    printf("Index: %d\n", idx);
    printf("Nazev: %s\n", arr[idx].name);
    printf("Cena: %.2f\n", arr[idx].price);
    printf("Kusy: %d\n", arr[idx].quantity);
    printf("Hodnota: %.2f\n", product_value(&arr[idx]));
}

static void search_by_name(const Product *arr, int count) {
    char name[NAME_LEN];
    if (!read_line("Zadej cely nazev produktu (presne): ", name, sizeof(name))) {
        printf("Input error.\n");
        return;
    }
    if (name[0] == '\0') {
        printf("Prazdny nazev.\n");
        return;
    }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (strcmp(arr[i].name, name) == 0) {
            printf("\nFound: [%d] %s | cena: %.2f | kusy: %d\n",
                   i, arr[i].name, arr[i].price, arr[i].quantity);
            found = 1;
        }
    }
    if (!found) {
        printf("\nProdukt nebyl nalezen: %s\n", name);
    }
}

static void search_by_price(const Product *arr, int count) {
    if (count == 0) {
        printf("\nSklad je prazdny.\n");
        return;
    }

    printf("\nSearch by price range.\n");
    double minv, maxv;
    if (!read_double("Min price: ", &minv)) {
        printf("Bad input.\n");
        return;
    }
    if (!read_double("Max price: ", &maxv)) {
        printf("Bad input.\n");
        return;
    }
    if (minv > maxv) {
        double tmp = minv;
        minv = maxv;
        maxv = tmp;
    }

    int found = 0;
    for (int i = 0; i < count; i++) {
        if (arr[i].price >= minv && arr[i].price <= maxv) {
            printf("[%d] %s | cena: %.2f | kusy: %d\n",
                   i, arr[i].name, arr[i].price, arr[i].quantity);
            found = 1;
        }
    }
    if (!found) {
        printf("No product in range %.2f .. %.2f\n", minv, maxv);
    }
}

static void delete_by_index(Product *arr, int *count) {
    if (*count == 0) {
        printf("\nSklad je prazdny.\n");
        return;
    }
    int idx;
    if (!read_int("Zadej index produktu k odstraneni: ", &idx)) {
        printf("Spatny vstup.\n");
        return;
    }
    if (idx < 0 || idx >= *count) {
        printf("Index mimo rozsah.\n");
        return;
    }

    printf("Removing: [%d] %s\n", idx, arr[idx].name);
    for (int i = idx; i < *count - 1; i++) {
        arr[i] = arr[i + 1];
    }
    (*count)--;
    printf("Done.\n");
}

static void edit_product(Product *arr, int count) {
    if (count == 0) {
        printf("\nSklad je prazdny.\n");
        return;
    }

    int idx;
    if (!read_int("Zadej index produktu k uprave: ", &idx)) {
        printf("Spatny vstup.\n");
        return;
    }
    if (idx < 0 || idx >= count) {
        printf("Index mimo rozsah.\n");
        return;
    }

    printf("\nEDIT PRODUCT [%d]\n", idx);
    printf("Current: %s | cena: %.2f | kusy: %d\n",
           arr[idx].name, arr[idx].price, arr[idx].quantity);
    printf("Leave empty and press ENTER to keep current value.\n");

    char buf[128];
    char name[NAME_LEN];

    snprintf(buf, sizeof(buf), "New name (current: %s): ", arr[idx].name);
    if (read_line(buf, name, sizeof(name))) {
        if (name[0] != '\0') {
            strncpy(arr[idx].name, name, NAME_LEN - 1);
            arr[idx].name[NAME_LEN - 1] = '\0';
        }
    }

    snprintf(buf, sizeof(buf), "New price (current: %.2f): ", arr[idx].price);
    char price_s[128];
    if (read_line(buf, price_s, sizeof(price_s))) {
        if (price_s[0] != '\0') {
            double p;
            if (parse_double(price_s, &p) && p >= 0) {
                arr[idx].price = p;
            } else {
                printf("Price not changed (invalid).\n");
            }
        }
    }

    snprintf(buf, sizeof(buf), "New quantity (current: %d): ", arr[idx].quantity);
    char qty_s[128];
    if (read_line(buf, qty_s, sizeof(qty_s))) {
        if (qty_s[0] != '\0') {
            int q;
            if (parse_int(qty_s, &q) && q >= 0) {
                arr[idx].quantity = q;
            } else {
                printf("Quantity not changed (invalid).\n");
            }
        }
    }

    printf("Updated: %s | cena: %.2f | kusy: %d\n",
           arr[idx].name, arr[idx].price, arr[idx].quantity);
}

static void add_product(Product *arr, int *count) {
    if (*count >= MAX_PRODUCTS) {
        printf("\nStock is full (max %d).\n", MAX_PRODUCTS);
        return;
    }

    Product p;
    char name[NAME_LEN];

    if (!read_line("New product name: ", name, sizeof(name)) || name[0] == '\0') {
        printf("Bad name.\n");
        return;
    }
    strncpy(p.name, name, NAME_LEN - 1);
    p.name[NAME_LEN - 1] = '\0';

    if (!read_double("Price: ", &p.price) || p.price < 0) {
        printf("Bad price.\n");
        return;
    }
    if (!read_int("Quantity: ", &p.quantity) || p.quantity < 0) {
        printf("Bad quantity.\n");
        return;
    }

    arr[*count] = p;
    (*count)++;
    printf("Added.\n");
}

static void save_to_file(const Product *arr, int count) {
    char path[256];
    if (!read_line("File name to save (e.g. sklad.txt): ", path, sizeof(path)) || path[0] == '\0') {
        printf("Bad path.\n");
        return;
    }

    FILE *f = fopen(path, "w");
    if (!f) {
        perror("Cannot open file");
        return;
    }

    for (int i = 0; i < count; i++) {
        fprintf(f, "%s;%.17g;%d\n", arr[i].name, arr[i].price, arr[i].quantity);
    }
    fclose(f);
    printf("Saved %d products to %s\n", count, path);
}

static void load_from_file(Product *arr, int *count) {
    char path[256];
    if (!read_line("File name to load: ", path, sizeof(path)) || path[0] == '\0') {
        printf("Bad path.\n");
        return;
    }

    FILE *f = fopen(path, "r");
    if (!f) {
        perror("Cannot open file");
        return;
    }

    Product tmp[MAX_PRODUCTS];
    int tmp_count = 0;
    char line[512];

    while (fgets(line, sizeof(line), f)) {
        trim_newline(line);
        if (line[0] == '\0') continue;

        char *name = strtok(line, ";");
        char *price_s = strtok(NULL, ";");
        char *qty_s = strtok(NULL, ";");

        if (!name || !price_s || !qty_s) {
            printf("Skipping bad line.\n");
            continue;
        }
        if (tmp_count >= MAX_PRODUCTS) {
            printf("Too many products in file. Rest ignored.\n");
            break;
        }

        Product p;
        strncpy(p.name, name, NAME_LEN - 1);
        p.name[NAME_LEN - 1] = '\0';

        if (!parse_double(price_s, &p.price) || p.price < 0) {
            printf("Skipping line with bad price.\n");
            continue;
        }
        if (!parse_int(qty_s, &p.quantity) || p.quantity < 0) {
            printf("Skipping line with bad quantity.\n");
            continue;
        }

        tmp[tmp_count++] = p;
    }
    fclose(f);

    for (int i = 0; i < tmp_count; i++) arr[i] = tmp[i];
    *count = tmp_count;
    printf("Loaded %d products from %s\n", *count, path);
}

static void show_stats(const Product *arr, int count) {
    double total = 0.0;
    int total_qty = 0;
    int low = 0;

    for (int i = 0; i < count; i++) {
        total += product_value(&arr[i]);
        total_qty += arr[i].quantity;
        if (arr[i].quantity < 5) low++;
    }

    printf("\nSTATS\n");
    printf("Product types: %d\n", count);
    printf("Total pieces: %d\n", total_qty);
    printf("Total value: %.2f\n", total);
    printf("Low stock (<5): %d\n", low);
}

void printMenu(void) {
    printf("\n================= SKLADOVE HOSPODARSTVI =================\n");
    printf("1) List products\n");
    printf("2) Search by exact name\n");
    printf("3) Search by price range\n");
    printf("4) Show product detail\n");
    printf("5) Delete product by index\n");
    printf("6) Edit product by index\n");
    printf("7) Add new product\n");
    printf("8) Save to file\n");
    printf("9) Load from file\n");
    printf("10) Stats\n");
    printf("X) Exit\n");
    printf("==========================================================\n");
    printf("Choice: ");
}

static void seed_demo_data(Product *arr, int *count) {
    Product demo[] = {
        {"Jablko", 12.50, 25},
        {"Mleko 1L", 24.90, 4},
        {"Chleb", 39.00, 7},
        {"Cokolada", 29.90, 3},
        {"Kava 250g", 119.00, 10}
    };

    int n = (int)(sizeof(demo) / sizeof(demo[0]));
    if (n > MAX_PRODUCTS) n = MAX_PRODUCTS;
    for (int i = 0; i < n; i++) arr[i] = demo[i];
    *count = n;
}

int main(void) {
    Product products[MAX_PRODUCTS];
    int count = 0;

    seed_demo_data(products, &count);

    char choice[32];
    for (;;) {
        printMenu();

        if (!read_line("", choice, sizeof(choice))) {
            printf("Input error.\n");
            break;
        }
        if (choice[0] == '\0') continue;

        if (strcmp(choice, "10") == 0) {
            show_stats(products, count);
            continue;
        }

        char c = (char)toupper((unsigned char)choice[0]);
        switch (c) {
            case '1': list_products(products, count); break;
            case '2': search_by_name(products, count); break;
            case '3': search_by_price(products, count); break;
            case '4': show_product_detail(products, count); break;
            case '5': delete_by_index(products, &count); break;
            case '6': edit_product(products, count); break;
            case '7': add_product(products, &count); break;
            case '8': save_to_file(products, count); break;
            case '9': load_from_file(products, &count); break;
            case 'S': show_stats(products, count); break;
            case 'X':
                printf("Exiting.\n");
                return 0;
            default:
                printf("Unknown option: %s\n", choice);
                break;
        }
    }
    return 0;
}
