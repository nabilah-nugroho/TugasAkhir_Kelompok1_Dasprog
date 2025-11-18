#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// --- KONFIGURASI ---
#define NAMA_FILE "data_tiket.txt"
#define MAX_NAMA 50
#define MAX_KATEGORI 20
#define KADALUARSA_DETIK (7 * 24 * 60 * 60) // 7 hari dalam detik

// --- STRUKTUR DATA TIKET ---
typedef struct {
    int id;
    char nama_konser[MAX_NAMA];
    char kategori[MAX_KATEGORI];
    float harga;
    int jumlah_stok;
    time_t waktu_dibuat; // Timestamp untuk update otomatis
} Tiket;

// --- GLOBAL VARIABLES (untuk manajemen memori) ---
Tiket *daftar_tiket = NULL;
int jumlah_tiket = 0;

// --- PROTOTIPE FUNGSI ---
void muat_data();
void simpan_data();
void tampilkan_tiket_detail(const Tiket *t);
int buat_id_unik();

// CRUD & Utility
void tambah_tiket();
void lihat_semua_tiket();
void cari_tiket();
void update_tiket();
void hapus_tiket();
void sorting_tiket();
void update_otomatis_kadaluarsa();

// --- FUNGSI UTILITY ---

// Konversi time_t ke string format
void time_to_str(time_t t, char *buffer, size_t buf_size) {
    strftime(buffer, buf_size, "%Y-%m-%d %H:%M:%S", localtime(&t));
}

// Menampilkan detail satu tiket
void tampilkan_tiket_detail(const Tiket *t) {
    char tgl_str[30];
    time_to_str(t->waktu_dibuat, tgl_str, sizeof(tgl_str));
    
    printf("---------------------------------\n");
    printf("  ID Tiket    : %d\n", t->id);
    printf("  Nama Konser : %s\n", t->nama_konser);
    printf("  Kategori    : %s\n", t->kategori);
    printf("  Harga       : Rp %.2f\n", t->harga);
    printf("  Jumlah      : %d\n", t->jumlah_stok);
    printf("  Tgl Dibuat  : %s\n", tgl_str);
}

// Mencari ID tertinggi untuk ID unik baru
int buat_id_unik() {
    int max_id = 0;
    for (int i = 0; i < jumlah_tiket; i++) {
        if (daftar_tiket[i].id > max_id) {
            max_id = daftar_tiket[i].id;
        }
    }
    return max_id + 1;
}

// --- FUNGSI I/O FILE (MEMBACA/MENYIMPAN) ---

void muat_data() {
    FILE *file = fopen(NAMA_FILE, "r");
    if (file == NULL) {
        printf("File %s tidak ditemukan. Membuat data baru.\n", NAMA_FILE);
        return;
    }

    int count = 0;
    Tiket temp_tiket;

    // Hitung jumlah baris/tiket
    while (fscanf(file, "%d;%[^;];%[^;];%f;%d;%ld\n", 
                  &temp_tiket.id, 
                  temp_tiket.nama_konser, 
                  temp_tiket.kategori, 
                  &temp_tiket.harga, 
                  &temp_tiket.jumlah_stok, 
                  &temp_tiket.waktu_dibuat) == 6) {
        count++;
    }

    if (count == 0) {
        fclose(file);
        return;
    }

    // Alokasi memori
    daftar_tiket = (Tiket *)malloc(count * sizeof(Tiket));
    if (daftar_tiket == NULL) {
        perror("Error alokasi memori");
        exit(EXIT_FAILURE);
    }
    jumlah_tiket = count;

    // Reset pointer file ke awal
    fseek(file, 0, SEEK_SET);

    // Baca data ke memori
    for (int i = 0; i < jumlah_tiket; i++) {
        fscanf(file, "%d;%[^;];%[^;];%f;%d;%ld\n", 
               &daftar_tiket[i].id, 
               daftar_tiket[i].nama_konser, 
               daftar_tiket[i].kategori, 
               &daftar_tiket[i].harga, 
               &daftar_tiket[i].jumlah_stok, 
               &daftar_tiket[i].waktu_dibuat);
    }
    
    fclose(file);
    printf("✅ %d tiket berhasil dimuat dari %s.\n", jumlah_tiket, NAMA_FILE);
}

void simpan_data() {
    FILE *file = fopen(NAMA_FILE, "w");
    if (file == NULL) {
        perror("Error membuka file untuk menyimpan");
        return;
    }

    for (int i = 0; i < jumlah_tiket; i++) {
        // Format: ID;NamaKonser;Kategori;Harga;Stok;Timestamp
        fprintf(file, "%d;%s;%s;%.2f;%d;%ld\n", 
                daftar_tiket[i].id, 
                daftar_tiket[i].nama_konser, 
                daftar_tiket[i].kategori, 
                daftar_tiket[i].harga, 
                daftar_tiket[i].jumlah_stok, 
                (long)daftar_tiket[i].waktu_dibuat);
    }

    fclose(file);
    printf("\n✅ Data berhasil disimpan ke %s.\n", NAMA_FILE);
}

// --- FUNGSI CRUD & TAMBAHAN ---

// 1. CREATE
void tambah_tiket() {
    printf("\n--- Tambah Tiket Baru ---\n");
    
    // Alokasi ulang memori untuk tiket baru
    Tiket *temp = (Tiket *)realloc(daftar_tiket, (jumlah_tiket + 1) * sizeof(Tiket));
    if (temp == NULL) {
        perror("Error re-alokasi memori");
        return;
    }
    daftar_tiket = temp;

    Tiket *new_tiket = &daftar_tiket[jumlah_tiket];

    new_tiket->id = buat_id_unik();
    printf("Nama Konser: ");
    scanf(" %49[^\n]", new_tiket->nama_konser);
    printf("Kategori (e.g., VIP, Reguler): ");
    scanf(" %19[^\n]", new_tiket->kategori);

    // Input Harga & Stok dengan validasi sederhana
    do {
        printf("Harga (Rp): ");
        if (scanf("%f", &new_tiket->harga) != 1 || new_tiket->harga < 0) {
            printf("Harga tidak valid. Masukkan angka positif.\n");
            while (getchar() != '\n'); // Bersihkan buffer
        } else {
            break;
        }
    } while (1);

    do {
        printf("Jumlah Stok: ");
        if (scanf("%d", &new_tiket->jumlah_stok) != 1 || new_tiket->jumlah_stok < 0) {
            printf("Stok tidak valid. Masukkan angka non-negatif.\n");
            while (getchar() != '\n'); // Bersihkan buffer
        } else {
            break;
        }
    } while (1);
    
    new_tiket->waktu_dibuat = time(NULL); // Catat waktu saat dibuat

    jumlah_tiket++;
    printf("\n✅ Tiket ID %d berhasil ditambahkan.\n", new_tiket->id);
}

// 2. READ
void lihat_semua_tiket() {
    printf("\n--- Daftar Semua Tiket ---\n");
    if (jumlah_tiket == 0) {
        printf("Belum ada data tiket.\n");
        return;
    }

    for (int i = 0; i < jumlah_tiket; i++) {
        tampilkan_tiket_detail(&daftar_tiket[i]);
    }
}

// 3. SEARCH
void cari_tiket() {
    if (jumlah_tiket == 0) {
        printf("\nBelum ada data tiket untuk dicari.\n");
        return;
    }
    
    char keyword[MAX_NAMA];
    printf("\n--- Cari Tiket ---\n");
    printf("Masukkan ID, Nama Konser, atau Kategori: ");
    scanf(" %49[^\n]", keyword);
    
    int ditemukan = 0;
    
    // Konversi keyword ke lowercase untuk pencarian non-case-sensitive
    char lower_keyword[MAX_NAMA];
    strcpy(lower_keyword, keyword);
    for(int i = 0; lower_keyword[i]; i++){
        lower_keyword[i] = tolower(lower_keyword[i]);
    }

    for (int i = 0; i < jumlah_tiket; i++) {
        char nama_lower[MAX_NAMA], kategori_lower[MAX_KATEGORI];
        
        // Konversi nama & kategori tiket ke lowercase
        strcpy(nama_lower, daftar_tiket[i].nama_konser);
        for(int j = 0; nama_lower[j]; j++) nama_lower[j] = tolower(nama_lower[j]);

        strcpy(kategori_lower, daftar_tiket[i].kategori);
        for(int j = 0; kategori_lower[j]; j++) kategori_lower[j] = tolower(kategori_lower[j]);

        // Cek
        if (daftar_tiket[i].id == atoi(keyword) || 
            strstr(nama_lower, lower_keyword) != NULL ||
            strstr(kategori_lower, lower_keyword) != NULL) 
        {
            tampilkan_tiket_detail(&daftar_tiket[i]);
            ditemukan++;
        }
    }

    if (ditemukan == 0) {
        printf("\n❌ Tiket tidak ditemukan.\n");
    } else {
        printf("\n✅ Ditemukan %d hasil.\n", ditemukan);
    }
}

// 4. UPDATE
void update_tiket() {
    lihat_semua_tiket();
    int id_update;
    printf("\nMasukkan ID Tiket yang ingin di-update: ");
    if (scanf("%d", &id_update) != 1) {
        printf("\n❌ Input ID tidak valid.\n");
        while (getchar() != '\n');
        return;
    }

    for (int i = 0; i < jumlah_tiket; i++) {
        if (daftar_tiket[i].id == id_update) {
            printf("\n--- Update Tiket ID %d ---\n", id_update);
            tampilkan_tiket_detail(&daftar_tiket[i]);
            
            // Nama Konser
            printf("Nama Konser baru (%s, ketik ENTER untuk skip): ", daftar_tiket[i].nama_konser);
            char temp_nama[MAX_NAMA];
            while (getchar() != '\n'); // Bersihkan buffer
            if (fgets(temp_nama, MAX_NAMA, stdin) != NULL && strlen(temp_nama) > 1) {
                temp_nama[strcspn(temp_nama, "\n")] = 0; // Hapus newline
                strcpy(daftar_tiket[i].nama_konser, temp_nama);
            }

            // Kategori
            printf("Kategori baru (%s, ketik ENTER untuk skip): ", daftar_tiket[i].kategori);
            char temp_kategori[MAX_KATEGORI];
            if (fgets(temp_kategori, MAX_KATEGORI, stdin) != NULL && strlen(temp_kategori) > 1) {
                temp_kategori[strcspn(temp_kategori, "\n")] = 0;
                strcpy(daftar_tiket[i].kategori, temp_kategori);
            }
            
            // Harga
            float new_harga;
            printf("Harga baru (%.2f, ketik 0 dan ENTER untuk skip): ", daftar_tiket[i].harga);
            if (scanf("%f", &new_harga) == 1 && new_harga > 0) {
                daftar_tiket[i].harga = new_harga;
            }
            
            // Stok
            int new_stok;
            printf("Jumlah Stok baru (%d, ketik -1 dan ENTER untuk skip): ", daftar_tiket[i].jumlah_stok);
            if (scanf("%d", &new_stok) == 1 && new_stok >= 0) {
                daftar_tiket[i].jumlah_stok = new_stok;
            }
            
            printf("\n✅ Tiket ID %d berhasil di-update.\n", id_update);
            return;
        }
    }
    
    printf("\n❌ Tiket dengan ID %d tidak ditemukan.\n", id_update);
}

// 5. DELETE
void hapus_tiket() {
    lihat_semua_tiket();
    int id_hapus;
    printf("\nMasukkan ID Tiket yang ingin dihapus: ");
    if (scanf("%d", &id_hapus) != 1) {
        printf("\n❌ Input ID tidak valid.\n");
        while (getchar() != '\n');
        return;
    }

    for (int i = 0; i < jumlah_tiket; i++) {
        if (daftar_tiket[i].id == id_hapus) {
            // Geser semua elemen setelah indeks i ke depan
            for (int j = i; j < jumlah_tiket - 1; j++) {
                daftar_tiket[j] = daftar_tiket[j + 1];
            }
            
            jumlah_tiket--;
            
            // Alokasi ulang memori (memperkecil ukuran array)
            if (jumlah_tiket == 0) {
                free(daftar_tiket);
                daftar_tiket = NULL;
            } else {
                Tiket *temp = (Tiket *)realloc(daftar_tiket, jumlah_tiket * sizeof(Tiket));
                if (temp == NULL) {
                    perror("Error re-alokasi memori setelah hapus");
                    // Sebaiknya program diakhiri atau ditangani lebih lanjut
                } else {
                    daftar_tiket = temp;
                }
            }

            printf("\n✅ Tiket ID %d berhasil dihapus.\n", id_hapus);
            return;
        }
    }

    printf("\n❌ Tiket dengan ID %d tidak ditemukan.\n", id_hapus);
}

// 6. SORTING (Menggunakan Bubble Sort sebagai contoh)
void sorting_tiket() {
    if (jumlah_tiket < 2) {
        printf("\nMinimal 2 tiket untuk melakukan sorting.\n");
        return;
    }
    
    int pilihan;
    printf("\n--- Sorting Tiket ---\n");
    printf("1. Harga (Termurah)\n");
    printf("2. Harga (Termahal)\n");
    printf("3. Nama Konser (A-Z)\n");
    printf("Pilih kriteria sorting (1-3): ");
    if (scanf("%d", &pilihan) != 1) {
        printf("\n❌ Pilihan tidak valid.\n");
        while (getchar() != '\n');
        return;
    }
    
    for (int i = 0; i < jumlah_tiket - 1; i++) {
        for (int j = 0; j < jumlah_tiket - i - 1; j++) {
            int tukar = 0;
            
            if (pilihan == 1 && daftar_tiket[j].harga > daftar_tiket[j+1].harga) tukar = 1;
            if (pilihan == 2 && daftar_tiket[j].harga < daftar_tiket[j+1].harga) tukar = 1;
            if (pilihan == 3 && strcmp(daftar_tiket[j].nama_konser, daftar_tiket[j+1].nama_konser) > 0) tukar = 1;
            
            if (tukar) {
                Tiket temp = daftar_tiket[j];
                daftar_tiket[j] = daftar_tiket[j+1];
                daftar_tiket[j+1] = temp;
            }
        }
    }
    
    printf("\n✅ Data berhasil diurutkan.\n");
    lihat_semua_tiket();
}

// 7. UPDATE OTOMATIS
void update_otomatis_kadaluarsa() {
    time_t sekarang = time(NULL);
    int tiket_diupdate = 0;
    
    for (int i = 0; i < jumlah_tiket; i++) {
        // Jika waktu sekarang dikurangi waktu dibuat lebih dari batas kadaluarsa
        if (sekarang - daftar_tiket[i].waktu_dibuat > KADALUARSA_DETIK) {
            if (daftar_tiket[i].jumlah_stok > 0) {
                daftar_tiket[i].jumlah_stok = 0;
                tiket_diupdate++;
            }
        }
    }

    if (tiket_diupdate > 0) {
        printf("\n⚠️  %d tiket otomatis di-update (kadaluarsa > 7 hari) menjadi 0 stok.\n", tiket_diupdate);
    }
}

// --- FUNGSI MENU UTAMA ---

void tampilkan_menu() {
    printf("\n===================================\n");
    printf("🎫 SISTEM PENJUALAN TIKET KONSER 🎫\n");
    printf("===================================\n");
    printf("1. Tambah Tiket Baru (Create)\n");
    printf("2. Lihat Semua Tiket (Read)\n");
    printf("3. Cari Tiket (Search)\n");
    printf("4. Update Detail Tiket\n");
    printf("5. Hapus Tiket (Delete)\n");
    printf("6. Sorting Tiket\n");
    printf("7. Simpan & Keluar\n");
    printf("-----------------------------------\n");
    printf("Pilihan Anda: ");
}

int main() {
    muat_data();
    update_otomatis_kadaluarsa(); // Jalankan update otomatis saat program dimulai

    int pilihan;
    
    do {
        tampilkan_menu();
        
        if (scanf("%d", &pilihan) != 1) {
            printf("\n❌ Input tidak valid. Masukkan angka 1-7.\n");
            while (getchar() != '\n'); // Bersihkan buffer input
            continue;
        }

        while (getchar() != '\n'); // Bersihkan buffer setelah scanf integer

        switch (pilihan) {
            case 1: tambah_tiket(); break;
            case 2: lihat_semua_tiket(); break;
            case 3: cari_tiket(); break;
            case 4: update_tiket(); break;
            case 5: hapus_tiket(); break;
            case 6: sorting_tiket(); break;
            case 7: 
                simpan_data(); 
                printf("Terima kasih, program diakhiri.\n");
                break;
            default:
                printf("\n❌ Pilihan tidak valid. Silakan coba lagi.\n");
                break;
        }
    } while (pilihan != 7);

    // Bebaskan memori sebelum keluar
    if (daftar_tiket != NULL) {
        free(daftar_tiket);
    }

    return 0;
}