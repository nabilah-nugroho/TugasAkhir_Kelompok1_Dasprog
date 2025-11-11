#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_WARGA 100
#define MAX_JALAN 5
#define MAX_NAMA 30

typedef struct {
    char nama[MAX_NAMA];
    int umur;
    float suhu;
    int polusi;
    int aman;
} Warga;

typedef struct {
    int statusLampu; // 0: merah, 1: hijau
    int kendaraan;
} Jalan;

void tampilkanLampuLaluLintas(Jalan *jalan, int n) {
    for(int i = 0; i < n; i++) {
        printf("Jalur %d: Lampu %s, Jumlah Kendaraan: %d\n", i+1, jalan[i].statusLampu ? "HIJAU" : "MERAH", jalan[i].kendaraan);
    }
}

void updateLampuOtomatis(Jalan *jalan, int n) {
    for(int i = 0; i < n; i++) {
        if(jalan[i].kendaraan > 5) jalan[i].statusLampu = 1; // jadi hijau
        else jalan[i].statusLampu = 0; // jadi merah
    }
}

// Fungsi rekursif untuk memperbarui umur warga tiap siklus
void tambahUmurWarga(Warga *warga, int idx, int jumlah) {
    if (idx == jumlah) return;
    warga[idx].umur++;
    tambahUmurWarga(warga, idx+1, jumlah);
}

void simpanLog(Warga *warga, int jumlah, Jalan *jalan, int nJalan, int siklus) {
    FILE *file = fopen("log_smartcity.txt", "a");
    if (file) {
        fprintf(file, "=== SIKLUS %d ===\n", siklus);
        for (int i = 0; i < jumlah; i++)
            fprintf(file, "Warga: %s, Umur: %d, Suhu: %.1f, Polusi: %d, Aman: %d\n",
                    warga[i].nama, warga[i].umur, warga[i].suhu, warga[i].polusi, warga[i].aman);
        for (int i = 0; i < nJalan; i++)
            fprintf(file, "Lampu Jalur %d: %s, Kendaraan: %d\n", i+1, jalan[i].statusLampu ? "HIJAU":"MERAH", jalan[i].kendaraan);
        fprintf(file, "\n");
        fclose(file);
    }
}

int main() {
    Warga daftarWarga[MAX_WARGA] = {{"Adi", 20, 35.2, 30, 1}, {"Budi", 35, 36.2, 40, 1}};
    Jalan jalanan[MAX_JALAN];
    int jumlahWarga = 2, siklus = 1, pilihan, keluar = 0;

    for(int i = 0; i < MAX_JALAN; i++) {
        jalanan[i].statusLampu = 0;
        jalanan[i].kendaraan = rand() % 10;
    }

    printf("===== MINI SMART CITY SIMULATOR =====\n");

    while(!keluar) {
        printf("\n-- SIKLUS %d --\n", siklus);
        printf("1. Pantau Lampu Lalu Lintas\n2. Update Warga & Suhu\n3. Simpan Log\n4. Keluar\nPilih: ");
        scanf("%d", &pilihan);

        switch(pilihan) {
            case 1:
                updateLampuOtomatis(jalanan, MAX_JALAN);
                tampilkanLampuLaluLintas(jalanan, MAX_JALAN);
                break;

            case 2:
                for(int i = 0; i < jumlahWarga; i++) {
                    daftarWarga[i].suhu += (rand() % 5 - 2) * 0.1;
                    daftarWarga[i].polusi += rand() % 10;
                    if(daftarWarga[i].polusi > 50) daftarWarga[i].aman = 0;
                }
                tambahUmurWarga(daftarWarga, 0, jumlahWarga);
                printf("Data warga diperbarui!\n");
                break;

            case 3:
                simpanLog(daftarWarga, jumlahWarga, jalanan, MAX_JALAN, siklus);
                printf("Log tersimpan!\n");
                break;

            case 4:
                keluar = 1;
                break;

            default:
                printf("Pilihan tidak tersedia\n");
        }
        siklus++;
    }
    printf("Terima kasih telah mencoba Smart City Simulator!\n");
    return 0;
}
