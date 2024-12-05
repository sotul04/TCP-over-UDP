# TCP Over UDP
## **Daftar Isi**

- [Author](#author)
- [Deskripsi Program](#deskripsi-program)
- [Tech Stack](#tech-stack)
- [Algoritma dan Penjelasannya](#algoritma-dan-penjelasannya)
- [Cara Menggunakan Program](#cara-menggunakan-program)

## **Author**

|   NIM    |           Nama           |
| :------: | :----------------------: |
| 13522062 | Salsabiila               |
| 13522093 | Matthew Vladimir Hutabarat |
| 13522098 | Suthasoma Mahardhika Munthe |
| 13522110 | Marvin Scifo Y. Hutahaean  |

## **Deskripsi Program**

<p align="justify">
Program ini adalah program yang mengirimkan User Datagram Protocol dengan cara-cara dan arsitektur dari Transmission Control Protocol

</p>

## **Tech Stack**
1. C++

## **Algoritma dan Penjelasannya**
Algoritma yang digunakan adalah sebagai berikut:
1. Three Way Handshake: Algoritma untuk memulai atau menginisiasi koneksi
2. Checksum: Algoritma untuk memastikan data yang dikirim bersih dan tidak rusak dengan menjumlahkan semua dan mencari nilai komplemen
3. CRC: Algoritma untuk memastikan data yang dikirim bersih dan tidak rusak dengan membagikan payload dengan CRC-16
4. GoBackN: Algoritma untuk mengirim data dari server ke client

## **Cara Menggunakan Program** ##
1. Run script.sh pada terminal jika file executable belum ada
2. Lalu, run file node dengan format ./node {ip} {port}
```console
./script.sh
./node {ip} {port}

// contoh
./node localhost 8080
```
2. Pada program terdapat dua argumen yaitu ip dan port. Jika ip tidak dimasukkan, ip akan di set sebagai localhost
3. Terdapat dua opsi yaitu Sender dan Receiver
4. Untuk Sender, akan diberikan opsi untuk mengirim ketikan CLI atau file
5. Untuk Receiver, akan ditanyakan port untuk mengambil data tersebut
6. Sender akan mengirimkan input ke receiver yang mempunyai port yang sama
7. Algoritma GoBackN akan digunakan untuk mengirim data tersebut sedikit demi sedikit
8. Data berhasil dikirim ke receiver

<!-- Optional -->
<!-- ## License -->
<!-- This project is open source and available under the [... License](). -->

<!-- You don't have to include all sections - just the one's relevant to your project -->
