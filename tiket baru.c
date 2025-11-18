#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define NAMA_FILE "data_tiket.txt"
#define FILE_PEMBELIAN "data_pembelian.txt" // File untuk menyimpan data pembelian
#define MAX_NAMA 50
#define MAX_KATEGORI 20
#define KADALUARSA_DETIK (7 * 24 * 60 * 60) // 7 hari dalam detik

// Struktur Tiket (tetap)
typedef struct {
    int id;
    char nama_konser[MAX_NAMA];
    char kategori[MAX_KATEGORI];
    float harga;
    int jumlah_stok;
    time_t waktu_dibuat;
} Tiket;

// Struktur Pembelian BARU
typedef struct {
    int id_pembelian;
    int id_tiket;
    int jumlah_beli;
    float total_harga;
    time_t waktu_pembelian;
} Pembelian;

// Variabel global
Tiket *daftar_tiket = NULL;
int jumlah_tiket = 0;

Pembelian *daftar_pembelian = NULL; // Daftar pembelian
int jumlah_pembelian = 0; // Jumlah transaksi pembelian

// Fungsi prototipe (tambahan untuk Pembelian)
void muat_data_pembelian();
void simpan_data_pembelian();
void beli_tiket();
void tampilkan_laporan_pembelian();
int buat_id_pembelian_unik();

// Fungsi prototipe (lama)
void muat_data();
void simpan_data();
void tampilkan_tiket_detail(const Tiket *t);
int buat_id_unik();
void tambah_tiket();
void lihat_semua_tiket();
void cari_tiket();
void update_tiket();
void hapus_tiket();
void sorting_tiket();
void update_otomatis_kadaluarsa();
void tampilkan_menu();
void bersihkan_buffer();
int login_admin();

// --- IMPLEMENTASI FUNGSI UTILITY ---

// Fungsi Utility untuk membersihkan newline
void bersihkan_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

// --- IMPLEMENTASI FUNGSI PEMBELIAN BARU ---

/**
 * @brief Memuat data pembelian dari file.
 */
void muat_data_pembelian() {
    FILE *file = fopen(FILE_PEMBELIAN, "r");
    if (file == NULL) {
        printf("⚠️ File data pembelian tidak ditemukan. Membuat file baru...\n");
        return;
    }

    int count = 0;
    Pembelian temp_pembelian;
    while (fread(&temp_pembelian, sizeof(Pembelian), 1, file) == 1) {
        count++;
    }

    if (count > 0) {
        daftar_pembelian = (Pembelian *)malloc(count * sizeof(Pembelian));
        if (daftar_pembelian == NULL) {
            perror("Gagal mengalokasikan memori saat memuat data pembelian");
            fclose(file);
            // Tidak exit, program utama masih bisa berjalan tanpa data pembelian
        } else {
            rewind(file);
            size_t items_read = fread(daftar_pembelian, sizeof(Pembelian), count, file);
            if (items_read != count) {
                fprintf(stderr, "Kesalahan saat membaca data pembelian dari file.\n");
                free(daftar_pembelian);
                daftar_pembelian = NULL;
                jumlah_pembelian = 0;
            } else {
                jumlah_pembelian = count;
                printf("✅ Berhasil memuat %d data pembelian dari file.\n", jumlah_pembelian);
            }
        }
    } else {
        printf("ℹ️ File data pembelian kosong.\n");
    }

    fclose(file);
}

/**
 * @brief Menyimpan data pembelian ke file.
 */
void simpan_data_pembelian() {
    FILE *file = fopen(FILE_PEMBELIAN, "w");
    if (file == NULL) {
        perror("❌ Gagal membuka file untuk menyimpan data pembelian");
        return;
    }

    if (jumlah_pembelian > 0) {
        size_t items_written = fwrite(daftar_pembelian, sizeof(Pembelian), jumlah_pembelian, file);
        if (items_written != jumlah_pembelian) {
            fprintf(stderr, "Kesalahan saat menulis data pembelian ke file.\n");
        } else {
            printf("✅ Berhasil menyimpan %d data pembelian ke file.\n", jumlah_pembelian);
        }
    } else {
        printf("ℹ️ Tidak ada data pembelian untuk disimpan.\n");
    }

    fclose(file);
}

/**
 * @brief Menghasilkan ID pembelian unik (berdasarkan ID maksimum + 1 atau 1 jika kosong).
 * @return ID unik baru.
 */
int buat_id_pembelian_unik() {
    if (jumlah_pembelian == 0) {
        return 1;
    }

    int max_id = 0;
    for (int i = 0; i < jumlah_pembelian; i++) {
        if (daftar_pembelian[i].id_pembelian > max_id) {
            max_id = daftar_pembelian[i].id_pembelian;
        }
    }
    return max_id + 1;
}

/**
 * @brief Melakukan pembelian tiket dan mengurangi stok.
 */
void beli_tiket() {
    int id_beli;
    int jumlah_beli;
    int index_tiket = -1;

    printf("\n🛒 --- BELI TIKET ---\n");

    if (jumlah_tiket == 0) {
        printf("⚠️ Saat ini tidak ada tiket yang tersedia untuk dijual.\n");
        return;
    }

    printf("Masukkan ID Tiket yang akan dibeli: ");
    if (scanf("%d", &id_beli) != 1) {
        printf("❌ ID tidak valid.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer();

    for (int i = 0; i < jumlah_tiket; i++) {
        if (daftar_tiket[i].id == id_beli) {
            index_tiket = i;
            break;
        }
    }

    if (index_tiket == -1) {
        printf("❌ Tiket dengan ID %d tidak ditemukan.\n", id_beli);
        return;
    }

    printf("\nDetail Tiket:\n");
    tampilkan_tiket_detail(&daftar_tiket[index_tiket]);

    printf("Masukkan Jumlah Tiket yang ingin dibeli: ");
    if (scanf("%d", &jumlah_beli) != 1 || jumlah_beli <= 0) {
        printf("❌ Jumlah pembelian tidak valid.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer();

    if (jumlah_beli > daftar_tiket[index_tiket].jumlah_stok) {
        printf("❌ Stok tidak cukup. Stok yang tersedia: %d\n", daftar_tiket[index_tiket].jumlah_stok);
        return;
    }

    // Lakukan Transaksi
    float total_harga = jumlah_beli * daftar_tiket[index_tiket].harga;

    // Update Stok
    daftar_tiket[index_tiket].jumlah_stok -= jumlah_beli;

    // Catat Pembelian
    Pembelian baru;
    baru.id_pembelian = buat_id_pembelian_unik();
    baru.id_tiket = id_beli;
    baru.jumlah_beli = jumlah_beli;
    baru.total_harga = total_harga;
    baru.waktu_pembelian = time(NULL);

    // Re-alokasi memori daftar pembelian
    Pembelian *temp = (Pembelian *)realloc(daftar_pembelian, (jumlah_pembelian + 1) * sizeof(Pembelian));
    if (temp == NULL) {
        perror("❌ Gagal mengalokasikan memori untuk data pembelian");
        // Rollback stok (penting jika alokasi memori gagal)
        daftar_tiket[index_tiket].jumlah_stok += jumlah_beli; 
        return;
    }
    daftar_pembelian = temp;
    daftar_pembelian[jumlah_pembelian] = baru;
    jumlah_pembelian++;

    printf("\n🎉 Transaksi berhasil!\n");
    printf("  | ID Pembelian: %d\n", baru.id_pembelian);
    printf("  | Tiket: %s (%s)\n", daftar_tiket[index_tiket].nama_konser, daftar_tiket[index_tiket].kategori);
    printf("  | Jumlah Beli: %d\n", baru.jumlah_beli);
    printf("  | Total Harga: Rp%.2f\n", baru.total_harga);
    printf("  | Stok Tersisa: %d\n", daftar_tiket[index_tiket].jumlah_stok);
    printf("-----------------------------------\n");

    // Simpan data tiket dan pembelian setelah transaksi
    simpan_data();
    simpan_data_pembelian();
}

/**
 * @brief Menampilkan laporan semua pembelian yang telah dilakukan.
 */
void tampilkan_laporan_pembelian() {
    printf("\n📊 --- LAPORAN PEMBELIAN (%d Transaksi) ---\n", jumlah_pembelian);

    if (jumlah_pembelian == 0) {
        printf("⚠️ Belum ada transaksi pembelian yang tercatat.\n");
        return;
    }

    float grand_total = 0.0;

    for (int i = 0; i < jumlah_pembelian; i++) {
        char waktu_str[64];
        struct tm *info_waktu = localtime(&daftar_pembelian[i].waktu_pembelian);
        strftime(waktu_str, sizeof(waktu_str), "%Y-%m-%d %H:%M:%S", info_waktu);

        // Cari nama konser yang sesuai
        char nama_tiket[MAX_NAMA] = "ID Tiket Tidak Ditemukan";
        for (int j = 0; j < jumlah_tiket; j++) {
            if (daftar_tiket[j].id == daftar_pembelian[i].id_tiket) {
                strcpy(nama_tiket, daftar_tiket[j].nama_konser);
                break;
            }
        }

        printf("--- Pembelian #%d ---\n", daftar_pembelian[i].id_pembelian);
        printf("  | Waktu: %s\n", waktu_str);
        printf("  | ID Tiket: %d (%s)\n", daftar_pembelian[i].id_tiket, nama_tiket);
        printf("  | Jumlah Beli: %d\n", daftar_pembelian[i].jumlah_beli);
        printf("  | Total Harga: Rp%.2f\n", daftar_pembelian[i].total_harga);
        printf("  +-------------------------\n");

        grand_total += daftar_pembelian[i].total_harga;
    }
    
    printf("\n💰 **GRAND TOTAL SEMUA PEMBELIAN: Rp%.2f**\n", grand_total);
    printf("--------------------------------------\n");
}


// --- FUNGSI LAINNYA (TIDAK BERUBAH SIGNIFIKAN) ---

/**
 * @brief Menampilkan detail satu tiket.
 * @param t Pointer ke struktur Tiket.
 */
void tampilkan_tiket_detail(const Tiket *t) {
    char waktu_str[64];
    struct tm *info_waktu = localtime(&t->waktu_dibuat);
    strftime(waktu_str, sizeof(waktu_str), "%Y-%m-%d %H:%M:%S", info_waktu);

    printf("  | ID: %d\n", t->id);
    printf("  | Nama Konser: %s\n", t->nama_konser);
    printf("  | Kategori: %s\n", t->kategori);
    printf("  | Harga: Rp%.2f\n", t->harga);
    printf("  | Stok: %d\n", t->jumlah_stok);
    printf("  | Waktu Dibuat: %s\n", waktu_str);
    printf("  +-----------------------------------\n");
}

/**
 * @brief Memuat data tiket dari file.
 */
void muat_data() {
    FILE *file = fopen(NAMA_FILE, "r");
    if (file == NULL) {
        printf("⚠️ File data tidak ditemukan. Membuat file baru...\n");
        return;
    }

    int count = 0;
    Tiket temp_tiket;
    while (fread(&temp_tiket, sizeof(Tiket), 1, file) == 1) {
        count++;
    }

    if (count > 0) {
        daftar_tiket = (Tiket *)malloc(count * sizeof(Tiket));
        if (daftar_tiket == NULL) {
            perror("Gagal mengalokasikan memori saat memuat data");
            fclose(file);
            exit(EXIT_FAILURE);
        }

        rewind(file);
        size_t items_read = fread(daftar_tiket, sizeof(Tiket), count, file);
        if (items_read != count) {
            fprintf(stderr, "Kesalahan saat membaca data dari file.\n");
            free(daftar_tiket);
            daftar_tiket = NULL;
            jumlah_tiket = 0;
        } else {
            jumlah_tiket = count;
            printf("✅ Berhasil memuat %d tiket dari file.\n", jumlah_tiket);
        }
    } else {
        printf("ℹ️ File data kosong.\n");
    }

    fclose(file);
}

/**
 * @brief Menyimpan data tiket ke file.
 */
void simpan_data() {
    FILE *file = fopen(NAMA_FILE, "w");
    if (file == NULL) {
        perror("❌ Gagal membuka file untuk menyimpan data");
        return;
    }

    if (jumlah_tiket > 0) {
        size_t items_written = fwrite(daftar_tiket, sizeof(Tiket), jumlah_tiket, file);
        if (items_written != jumlah_tiket) {
            fprintf(stderr, "Kesalahan saat menulis data ke file.\n");
        } else {
            printf("✅ Berhasil menyimpan %d tiket ke file.\n", jumlah_tiket);
        }
    } else {
        printf("ℹ️ Tidak ada tiket untuk disimpan.\n");
    }

    fclose(file);
}

/**
 * @brief Menghasilkan ID unik (berdasarkan ID maksimum + 1 atau 1 jika kosong).
 * @return ID unik baru.
 */
int buat_id_unik() {
    if (jumlah_tiket == 0) {
        return 1;
    }

    int max_id = 0;
    for (int i = 0; i < jumlah_tiket; i++) {
        if (daftar_tiket[i].id > max_id) {
            max_id = daftar_tiket[i].id;
        }
    }
    return max_id + 1;
}

/**
 * @brief Menambahkan tiket baru ke sistem.
 */
void tambah_tiket() {
    Tiket baru;
    char buffer[100];

    printf("\n➕ --- TAMBAH TIKET BARU ---\n");

    // Dapatkan ID unik
    baru.id = buat_id_unik();
    printf("  ID Tiket Baru: %d\n", baru.id);

    // Input Nama Konser
    printf("  Masukkan Nama Konser (Maks %d karakter): ", MAX_NAMA - 1);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    buffer[strcspn(buffer, "\n")] = 0; // Hapus newline
    strncpy(baru.nama_konser, buffer, MAX_NAMA - 1);
    baru.nama_konser[MAX_NAMA - 1] = '\0';

    // Input Kategori
    printf("  Masukkan Kategori (VIP, Reguler, dsb. Maks %d karakter): ", MAX_KATEGORI - 1);
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    buffer[strcspn(buffer, "\n")] = 0; // Hapus newline
    strncpy(baru.kategori, buffer, MAX_KATEGORI - 1);
    baru.kategori[MAX_KATEGORI - 1] = '\0';

    // Input Harga
    printf("  Masukkan Harga Tiket (Contoh: 150000.00): Rp");
    if (scanf("%f", &baru.harga) != 1 || baru.harga < 0) {
        printf("❌ Harga tidak valid.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer(); // Bersihkan buffer

    // Input Jumlah Stok
    printf("  Masukkan Jumlah Stok Tiket: ");
    if (scanf("%d", &baru.jumlah_stok) != 1 || baru.jumlah_stok < 0) {
        printf("❌ Jumlah stok tidak valid.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer(); // Bersihkan buffer

    // Set waktu pembuatan
    baru.waktu_dibuat = time(NULL);

    // Re-alokasi memori
    Tiket *temp = (Tiket *)realloc(daftar_tiket, (jumlah_tiket + 1) * sizeof(Tiket));
    if (temp == NULL) {
        perror("❌ Gagal mengalokasikan memori untuk tiket baru");
        return;
    }
    daftar_tiket = temp;
    daftar_tiket[jumlah_tiket] = baru;
    jumlah_tiket++;

    printf("\n🎉 Tiket berhasil ditambahkan:\n");
    tampilkan_tiket_detail(&baru);
}

/**
 * @brief Menampilkan semua tiket yang ada.
 */
void lihat_semua_tiket() {
    printf("\n📚 --- SEMUA DAFTAR TIKET (%d Tiket) ---\n", jumlah_tiket);

    if (jumlah_tiket == 0) {
        printf("⚠️ Saat ini tidak ada tiket dalam sistem.\n");
        return;
    }

    for (int i = 0; i < jumlah_tiket; i++) {
        printf("--- Tiket #%d ---\n", i + 1);
        tampilkan_tiket_detail(&daftar_tiket[i]);
    }
    printf("--------------------------------------\n");
}

/**
 * @brief Mencari tiket berdasarkan ID, Nama Konser, atau Kategori.
 */
void cari_tiket() {
    int pilihan_cari;
    int id_cari;
    char kriteria_cari[MAX_NAMA];
    int ditemukan = 0;

    printf("\n🔍 --- CARI TIKET ---\n");
    printf("Cari berdasarkan:\n");
    printf("1. ID Tiket\n");
    printf("2. Nama Konser\n");
    printf("3. Kategori\n");
    printf("Pilih opsi (1-3): ");

    if (scanf("%d", &pilihan_cari) != 1) {
        printf("❌ Input tidak valid.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer();

    if (jumlah_tiket == 0) {
        printf("⚠️ Tidak ada tiket dalam sistem untuk dicari.\n");
        return;
    }

    switch (pilihan_cari) {
        case 1:
            printf("Masukkan ID Tiket yang dicari: ");
            if (scanf("%d", &id_cari) != 1) {
                printf("❌ ID tidak valid.\n");
                bersihkan_buffer();
                return;
            }
            bersihkan_buffer();

            printf("\nHasil Pencarian ID %d:\n", id_cari);
            for (int i = 0; i < jumlah_tiket; i++) {
                if (daftar_tiket[i].id == id_cari) {
                    tampilkan_tiket_detail(&daftar_tiket[i]);
                    ditemukan = 1;
                    break;
                }
            }
            break;

        case 2:
            printf("Masukkan Nama Konser yang dicari: ");
            if (fgets(kriteria_cari, sizeof(kriteria_cari), stdin) == NULL) return;
            kriteria_cari[strcspn(kriteria_cari, "\n")] = 0;

            printf("\nHasil Pencarian Nama Konser '%s':\n", kriteria_cari);
            for (int i = 0; i < jumlah_tiket; i++) {
                char temp_nama[MAX_NAMA], temp_kriteria[MAX_NAMA];
                strcpy(temp_nama, daftar_tiket[i].nama_konser);
                strcpy(temp_kriteria, kriteria_cari);
                for(int j = 0; temp_nama[j]; j++){ temp_nama[j] = tolower(temp_nama[j]); }
                for(int j = 0; temp_kriteria[j]; j++){ temp_kriteria[j] = tolower(temp_kriteria[j]); }

                if (strstr(temp_nama, temp_kriteria) != NULL) {
                    tampilkan_tiket_detail(&daftar_tiket[i]);
                    ditemukan = 1;
                }
            }
            break;

        case 3:
            printf("Masukkan Kategori yang dicari: ");
            if (fgets(kriteria_cari, sizeof(kriteria_cari), stdin) == NULL) return;
            kriteria_cari[strcspn(kriteria_cari, "\n")] = 0;

            printf("\nHasil Pencarian Kategori '%s':\n", kriteria_cari);
            for (int i = 0; i < jumlah_tiket; i++) {
                char temp_kategori[MAX_KATEGORI], temp_kriteria[MAX_KATEGORI];
                strcpy(temp_kategori, daftar_tiket[i].kategori);
                strcpy(temp_kriteria, kriteria_cari);
                for(int j = 0; temp_kategori[j]; j++){ temp_kategori[j] = tolower(temp_kategori[j]); }
                for(int j = 0; temp_kriteria[j]; j++){ temp_kriteria[j] = tolower(temp_kriteria[j]); }

                if (strcmp(temp_kategori, temp_kriteria) == 0) {
                    tampilkan_tiket_detail(&daftar_tiket[i]);
                    ditemukan = 1;
                }
            }
            break;

        default:
            printf("❌ Pilihan pencarian tidak valid.\n");
            return;
    }

    if (!ditemukan) {
        printf("⚠️ Tiket tidak ditemukan.\n");
    }
}

/**
 * @brief Mengupdate informasi tiket yang sudah ada.
 */
void update_tiket() {
    int id_update;
    int index_update = -1;
    char buffer[100];

    printf("\n📝 --- UPDATE TIKET ---\n");

    if (jumlah_tiket == 0) {
        printf("⚠️ Tidak ada tiket dalam sistem untuk diupdate.\n");
        return;
    }

    printf("Masukkan ID Tiket yang akan diupdate: ");
    if (scanf("%d", &id_update) != 1) {
        printf("❌ ID tidak valid.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer();

    for (int i = 0; i < jumlah_tiket; i++) {
        if (daftar_tiket[i].id == id_update) {
            index_update = i;
            break;
        }
    }

    if (index_update == -1) {
        printf("❌ Tiket dengan ID %d tidak ditemukan.\n", id_update);
        return;
    }

    printf("\nTiket yang akan diupdate:\n");
    tampilkan_tiket_detail(&daftar_tiket[index_update]);
    printf("-----------------------------------\n");

    // Update Nama Konser
    printf("  Masukkan Nama Konser Baru (Kosongkan untuk tidak diubah): ");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) {
        strncpy(daftar_tiket[index_update].nama_konser, buffer, MAX_NAMA - 1);
        daftar_tiket[index_update].nama_konser[MAX_NAMA - 1] = '\0';
    }

    // Update Kategori
    printf("  Masukkan Kategori Baru (Kosongkan untuk tidak diubah): ");
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) return;
    buffer[strcspn(buffer, "\n")] = 0;
    if (strlen(buffer) > 0) {
        strncpy(daftar_tiket[index_update].kategori, buffer, MAX_KATEGORI - 1);
        daftar_tiket[index_update].kategori[MAX_KATEGORI - 1] = '\0';
    }

    // Update Harga
    printf("  Masukkan Harga Tiket Baru (0 untuk tidak diubah): Rp");
    float harga_baru;
    if (scanf("%f", &harga_baru) == 1 && harga_baru > 0) {
        daftar_tiket[index_update].harga = harga_baru;
    }
    bersihkan_buffer();

    // Update Jumlah Stok
    printf("  Masukkan Jumlah Stok Tiket Baru (-1 untuk tidak diubah): ");
    int stok_baru;
    if (scanf("%d", &stok_baru) == 1 && stok_baru >= 0) {
        daftar_tiket[index_update].jumlah_stok = stok_baru;
    }
    bersihkan_buffer();

    // Update waktu_dibuat juga direset agar kadaluarsa dihitung ulang (opsional)
    char konfirmasi;
    printf("Perbarui waktu kadaluarsa (Y/T)? ");
    if (scanf(" %c", &konfirmasi) == 1 && (konfirmasi == 'Y' || konfirmasi == 'y')) {
        daftar_tiket[index_update].waktu_dibuat = time(NULL);
        printf("Waktu pembuatan diperbarui.\n");
    }
    bersihkan_buffer();


    printf("\n✅ Tiket ID %d berhasil diupdate.\n", id_update);
    tampilkan_tiket_detail(&daftar_tiket[index_update]);
}

/**
 * @brief Menghapus tiket berdasarkan ID.
 */
void hapus_tiket() {
    int id_hapus;
    int index_hapus = -1;
    char konfirmasi;

    printf("\n🗑️ --- HAPUS TIKET ---\n");

    if (jumlah_tiket == 0) {
        printf("⚠️ Tidak ada tiket dalam sistem untuk dihapus.\n");
        return;
    }

    printf("Masukkan ID Tiket yang akan dihapus: ");
    if (scanf("%d", &id_hapus) != 1) {
        printf("❌ ID tidak valid.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer();

    for (int i = 0; i < jumlah_tiket; i++) {
        if (daftar_tiket[i].id == id_hapus) {
            index_hapus = i;
            break;
        }
    }

    if (index_hapus == -1) {
        printf("❌ Tiket dengan ID %d tidak ditemukan.\n", id_hapus);
        return;
    }

    printf("\nAnda yakin ingin menghapus tiket berikut?\n");
    tampilkan_tiket_detail(&daftar_tiket[index_hapus]);
    printf("Konfirmasi (Y/T): ");
    if (scanf(" %c", &konfirmasi) != 1 || (konfirmasi != 'Y' && konfirmasi != 'y')) {
        printf("Pembatalan penghapusan.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer();

    // Geser elemen setelah yang dihapus
    for (int i = index_hapus; i < jumlah_tiket - 1; i++) {
        daftar_tiket[i] = daftar_tiket[i + 1];
    }
    jumlah_tiket--;

    // Re-alokasi memori
    if (jumlah_tiket == 0) {
        free(daftar_tiket);
        daftar_tiket = NULL;
    } else {
        Tiket *temp = (Tiket *)realloc(daftar_tiket, jumlah_tiket * sizeof(Tiket));
        if (temp == NULL) {
            perror("Gagal re-alokasi memori setelah penghapusan");
        } else {
            daftar_tiket = temp;
        }
    }

    printf("✅ Tiket ID %d berhasil dihapus.\n", id_hapus);
}

/**
 * @brief Fungsi perbandingan untuk qsort (berdasarkan harga).
 */
int bandingkan_harga(const void *a, const void *b) {
    const Tiket *tiket_a = (const Tiket *)a;
    const Tiket *tiket_b = (const Tiket *)b;
    if (tiket_a->harga < tiket_b->harga) return -1;
    if (tiket_a->harga > tiket_b->harga) return 1;
    return 0;
}

/**
 * @brief Fungsi perbandingan untuk qsort (berdasarkan nama konser).
 */
int bandingkan_nama(const void *a, const void *b) {
    const Tiket *tiket_a = (const Tiket *)a;
    const Tiket *tiket_b = (const Tiket *)b;
    return strcmp(tiket_a->nama_konser, tiket_b->nama_konser);
}

/**
 * @brief Mengurutkan tiket berdasarkan kriteria.
 */
void sorting_tiket() {
    int pilihan_sort;

    printf("\n➡️ --- URUTKAN TIKET ---\n");

    if (jumlah_tiket < 2) {
        printf("⚠️ Minimal diperlukan 2 tiket untuk melakukan pengurutan.\n");
        return;
    }

    printf("Urutkan berdasarkan:\n");
    printf("1. Harga (Termurah ke Termahal)\n");
    printf("2. Nama Konser (A-Z)\n");
    printf("Pilih opsi (1-2): ");

    if (scanf("%d", &pilihan_sort) != 1) {
        printf("❌ Input tidak valid.\n");
        bersihkan_buffer();
        return;
    }
    bersihkan_buffer();

    switch (pilihan_sort) {
        case 1:
            qsort(daftar_tiket, jumlah_tiket, sizeof(Tiket), bandingkan_harga);
            printf("✅ Tiket berhasil diurutkan berdasarkan Harga.\n");
            lihat_semua_tiket();
            break;
        case 2:
            qsort(daftar_tiket, jumlah_tiket, sizeof(Tiket), bandingkan_nama);
            printf("✅ Tiket berhasil diurutkan berdasarkan Nama Konser.\n");
            lihat_semua_tiket();
            break;
        default:
            printf("❌ Pilihan pengurutan tidak valid.\n");
            break;
    }
}

/**
 * @brief Menghapus tiket yang sudah kadaluarsa (lebih dari KADALUARSA_DETIK).
 */
void update_otomatis_kadaluarsa() {
    if (jumlah_tiket == 0) {
        return;
    }

    time_t waktu_sekarang = time(NULL);
    int tiket_dihapus = 0;
    int i = 0;

    printf("\n⏳ Pemeriksaan tiket kadaluarsa...\n");

    while (i < jumlah_tiket) {
        if (waktu_sekarang - daftar_tiket[i].waktu_dibuat > KADALUARSA_DETIK) {
            // Tiket kadaluarsa, hapus
            printf("  🗑️ Tiket kadaluarsa ditemukan dan dihapus: ID %d - %s\n",
                       daftar_tiket[i].id, daftar_tiket[i].nama_konser);

            // Geser elemen setelah yang dihapus
            for (int j = i; j < jumlah_tiket - 1; j++) {
                daftar_tiket[j] = daftar_tiket[j + 1];
            }
            jumlah_tiket--;
            tiket_dihapus++;
            // Jangan increment i, karena elemen berikutnya sekarang ada di posisi i
        } else {
            i++; // Lanjut ke elemen berikutnya
        }
    }

    if (tiket_dihapus > 0) {
        // Re-alokasi memori setelah semua penghapusan
        if (jumlah_tiket == 0) {
            free(daftar_tiket);
            daftar_tiket = NULL;
        } else {
            Tiket *temp = (Tiket *)realloc(daftar_tiket, jumlah_tiket * sizeof(Tiket));
            if (temp != NULL) {
                daftar_tiket = temp;
            } else {
                perror("Gagal re-alokasi memori setelah penghapusan kadaluarsa");
            }
        }
        printf("✅ Total %d tiket kadaluarsa dihapus.\n", tiket_dihapus);
        simpan_data(); // Simpan perubahan setelah penghapusan otomatis
    } else {
        printf("✅ Tidak ada tiket yang kadaluarsa.\n");
    }
}

/**
 * @brief Menampilkan menu utama.
 */
void tampilkan_menu() {
    printf("\n====================================\n");
    printf("🎟️ Sistem Manajemen Tiket UPNVJ 🎟️\n");
    printf("====================================\n");
    printf("1. Tambah Tiket Baru\n");
    printf("2. Lihat Semua Tiket\n");
    printf("3. Cari Tiket\n");
    printf("4. Update Informasi Tiket\n");
    printf("5. Hapus Tiket\n");
    printf("6. Urutkan Tiket\n");
    printf("--- Fungsionalitas Tambahan ---\n");
    printf("8. **Beli Tiket (Kurangi Stok)**\n"); // Tambahan
    printf("9. **Laporan Pembelian**\n"); // Tambahan
    printf("7. Keluar & Simpan\n");
    printf("------------------------------------\n");
    printf("Pilih opsi (1-9): ");
}


// Fungsi login administrator (Diperbaiki I/O)
int login_admin() {
    char username[20];
    int password;

    printf("Selamat datang di ruang administrator TIXUPNVJ!\n");
    printf("Halaman ini hanya diperuntukan untuk administrator sistem.\n");
    printf("Administrator/Pelanggan\n");
    printf("Masukkan username administrator: ");
    
    // Perbaikan: Menggunakan scanf dan kemudian membersihkan buffer
    if (scanf("%19s", username) != 1) return 0;
    bersihkan_buffer();

    if (strcmp(username, "NabilahArkanKey") == 0) {
        printf("Masukkan password: ");
        if (scanf("%d", &password) != 1) {
            printf("Password tidak valid.\n");
            bersihkan_buffer();
            return 0;
        }
        bersihkan_buffer();

        if (password == 2025) {
            printf("Akses diterima! Selamat datang, Administrator %s\n", username);
            return 1;  // Login berhasil
        } else {
            printf("Password salah! Akses ditolak.\n");
            return 0;
        }
    } else {
        printf("Username salah! Akses ditolak.\n");
        return 0;
    }
}

// Fungsi Main (Diperbaiki I/O)
int main() {
    // Cek login admin dulu
    if (!login_admin()) {
        printf("Keluar dari program.\n");
        return 0;
    }

    // Muat semua data
    muat_data();
    muat_data_pembelian(); // Muat data pembelian

    update_otomatis_kadaluarsa();

    int pilihan;
    do {
        tampilkan_menu();

        if (scanf("%d", &pilihan) != 1) {
            printf("\n❌ Input tidak valid. Masukkan angka 1-9.\n");
            bersihkan_buffer();
            continue;
        }
        bersihkan_buffer();

        switch (pilihan) {
            case 1: tambah_tiket(); break;
            case 2: lihat_semua_tiket(); break;
            case 3: cari_tiket(); break;
            case 4: update_tiket(); break;
            case 5: hapus_tiket(); break;
            case 6: sorting_tiket(); break;
            case 8: beli_tiket(); break; // Fungsi BARU: Beli Tiket
            case 9: tampilkan_laporan_pembelian(); break; // Fungsi BARU: Laporan Pembelian
            case 7:
                simpan_data();
                simpan_data_pembelian(); // Simpan data pembelian
                printf("Terima kasih, program diakhiri.\n");
                break;
            default:
                printf("\n❌ Pilihan tidak valid. Silakan coba lagi.\n");
                break;
        }
    } while (pilihan != 7);

    // Bebaskan memori
    if (daftar_tiket != NULL) {
        free(daftar_tiket);
    }
    if (daftar_pembelian != NULL) { // Bebaskan memori pembelian
        free(daftar_pembelian);
    }

    return 0;
}