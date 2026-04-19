// Лабораторна робота №2. Побітові операції.
// Варіант 14
//
// Завдання 1: 2049*b + (d*15+12*a)/2048 - c*100 + d*104
// Завдання 2: Шифрування 16 рядків по 4 символи (побітові операції)
// Завдання 3: Те саме через структури з бітовими полями та union

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// ============================================================
// Завдання 1
// 2049*b + (d*15+12*a)/2048 - c*100 + d*104  без * та /
//   2049*b  = (b<<11) + b
//   15*d    = (d<<4) - d
//   12*a    = (a<<3) + (a<<2)
//   /2048   = >>11
//   100*c   = (c<<6) + (c<<5) + (c<<2)
//   104*d   = (d<<6) + (d<<5) + (d<<3)
// ============================================================
void task1() {
    long long a, b, c, d;
    cout << "--- Завдання 1 ---\nВведіть a, b, c, d: ";
    cin >> a >> b >> c >> d;

    long long x = (b << 11) + b
        + (((d << 4) - d + (a << 3) + (a << 2)) >> 11)
        - (c << 6) - (c << 5) - (c << 2)
        + (d << 6) + (d << 5) + (d << 3);

    cout << "Результат: " << x << endl;
}

// ============================================================
// Завдання 2 — Шифрування/розшифрування через побітові операції
//
// Структура двох байтів (16 біт), варіант 14:
//   біти  0- 3 : номер рядка (4 біти)
//   біти  4- 7 : старша частина ASCII (біти 4-7 символу)
//   біт   8    : біт парності полів 0-7
//   біти  9-12 : молодша частина ASCII (біти 0-3 символу)
//   біти 13-14 : позиція символу в рядку (2 біти)
//   біт  15    : біт парності полів 8-14
// ============================================================

// Обчислення XOR-парності бітів [from, to)
static unsigned short parity_range(unsigned short val, int from, int to) {
    unsigned short p = 0;
    for (int i = from; i < to; i++) p ^= (val >> i) & 1;
    return p;
}

// Зашифрувати один символ
static unsigned short encode_char(unsigned char ch, int row, int pos) {
    unsigned short r = 0;
    r |= (unsigned short)(row & 0x0F);              // біти 0-3
    r |= ((ch >> 4) & 0x0F) << 4;                  // біти 4-7
    r |= parity_range(r, 0, 8) << 8;               // біт 8
    r |= (unsigned short)(ch & 0x0F) << 9;         // біти 9-12
    r |= (unsigned short)(pos & 0x03) << 13;        // біти 13-14
    r |= parity_range(r, 8, 15) << 15;             // біт 15
    return r;
}

// Розшифрувати один символ; повертає false при помилці парності
static bool decode_char(unsigned short r, unsigned char& ch, int& row, int& pos) {
    row = r & 0x0F;
    pos = (r >> 13) & 0x03;
    ch = (unsigned char)((((r >> 4) & 0x0F) << 4) | ((r >> 9) & 0x0F));

    if (parity_range(r, 0, 8) != ((r >> 8) & 1)) return false;
    if (parity_range(r, 8, 15) != ((r >> 15) & 1)) return false;
    return true;
}

void task2() {
    cout << "--- Завдання 2 (побітові операції) ---\n";
    cout << "Введіть 16 рядків по 4 символи:\n";
    cin.ignore();

    const int ROWS = 16, COLS = 4;
    unsigned short encoded[ROWS * COLS];

    // Шифрування
    for (int i = 0; i < ROWS; i++) {
        cout << "Рядок " << i + 1 << ": ";
        string line;
        getline(cin, line);
        while ((int)line.size() < COLS) line += ' ';
        for (int j = 0; j < COLS; j++)
            encoded[i * COLS + j] = encode_char((unsigned char)line[j], i, j);
    }

    // Запис у бінарний файл
    ofstream ofs("encoded2.bin", ios::binary);
    ofs.write((char*)encoded, sizeof(encoded));
    ofs.close();
    cout << "Записано у encoded2.bin\n";

    // Читання та розшифрування
    unsigned short buf[ROWS * COLS];
    ifstream ifs("encoded2.bin", ios::binary);
    ifs.read((char*)buf, sizeof(buf));
    ifs.close();

    cout << "Розшифрований текст:\n";
    bool ok = true;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            unsigned char ch; int row, pos;
            if (!decode_char(buf[i * COLS + j], ch, row, pos)) ok = false;
            cout << ch;
        }
        cout << "\n";
    }
    cout << (ok ? "Парність: OK\n" : "Є помилки парності!\n");
}

// ============================================================
// Завдання 3 — Те саме через структури з бітовими полями та union
// ============================================================

struct zsuv {
    unsigned short row : 4;  // біти 0-3
    unsigned short high_ascii : 4;  // біти 4-7
    unsigned short parity1 : 1;  // біт 8
    unsigned short low_ascii : 4;  // біти 9-12
    unsigned short pos : 2;  // біти 13-14
    unsigned short parity2 : 1;  // біт 15
};

union MyData {
    zsuv           bits;
    unsigned short word;
};

static unsigned short encode_char_struct(unsigned char ch, int row, int pos) {
    MyData d;
    d.word = 0;
    d.bits.row = row & 0x0F;
    d.bits.high_ascii = (ch >> 4) & 0x0F;
    d.bits.low_ascii = ch & 0x0F;
    d.bits.pos = pos & 0x03;
    d.bits.parity1 = parity_range(d.word, 0, 8);
    d.bits.parity2 = parity_range(d.word, 8, 15);
    return d.word;
}

static bool decode_char_struct(unsigned short word, unsigned char& ch, int& row, int& pos) {
    MyData d;
    d.word = word;
    row = d.bits.row;
    pos = d.bits.pos;
    ch = (unsigned char)((d.bits.high_ascii << 4) | d.bits.low_ascii);

    if (parity_range(word, 0, 8) != d.bits.parity1) return false;
    if (parity_range(word, 8, 15) != d.bits.parity2) return false;
    return true;
}

void task3() {
    cout << "--- Завдання 3 (структури + union) ---\n";
    cout << "Введіть 16 рядків по 4 символи:\n";
    if (cin.peek() == '\n') cin.ignore();

    const int ROWS = 16, COLS = 4;
    unsigned short encoded[ROWS * COLS];

    for (int i = 0; i < ROWS; i++) {
        cout << "Рядок " << i + 1 << ": ";
        string line;
        getline(cin, line);
        while ((int)line.size() < COLS) line += ' ';
        for (int j = 0; j < COLS; j++)
            encoded[i * COLS + j] = encode_char_struct((unsigned char)line[j], i, j);
    }

    ofstream ofs("encoded3.bin", ios::binary);
    ofs.write((char*)encoded, sizeof(encoded));
    ofs.close();
    cout << "Записано у encoded3.bin\n";

    unsigned short buf[ROWS * COLS];
    ifstream ifs("encoded3.bin", ios::binary);
    ifs.read((char*)buf, sizeof(buf));
    ifs.close();

    cout << "Розшифрований текст:\n";
    bool ok = true;
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            unsigned char ch; int row, pos;
            if (!decode_char_struct(buf[i * COLS + j], ch, row, pos)) ok = false;
            cout << ch;
        }
        cout << "\n";
    }
    cout << (ok ? "Парність: OK\n" : "Є помилки парності!\n");
}

// ============================================================
// Завдання 4 — CRC перевірка (варіант 2)
// 14-й біт — XOR непарних бітів (1,3,5,...,13)
// 15-й біт — XOR парних бітів  (0,2,4,...,12)
// ============================================================
void task4() {
    unsigned short word;
    cout << "--- Завдання 4 (CRC перевірка) ---\nВведіть 16-бітне число (hex): ";
    cin >> hex >> word;
    cout << dec;

    unsigned short crc_odd = 0, crc_even = 0;
    for (int i = 0; i < 14; i++) {
        if (i % 2 == 1) crc_odd ^= (word >> i) & 1;
        else            crc_even ^= (word >> i) & 1;
    }

    bool bit14 = (word >> 14) & 1;
    bool bit15 = (word >> 15) & 1;

    if (bit14 == crc_odd && bit15 == crc_even)
        cout << "Передача успішна: CRC збігається.\n";
    else
        cout << "Помилка: CRC не збігається!\n";
}

int main() {
    int choice;
    do {
        cout << "\n========== Меню ==========\n"
            << "1. Завдання 1 (побітові вирази)\n"
            << "2. Завдання 2 (шифрування, побітові операції)\n"
            << "3. Завдання 3 (шифрування, бітові структури)\n"
            << "4. Завдання 4 (CRC перевірка)\n"
            << "0. Вихід\n"
            << "Вибір: ";
        cin >> choice;
        switch (choice) {
        case 1: task1(); break;
        case 2: task2(); break;
        case 3: task3(); break;
        case 4: task4(); break;
        case 0: cout << "До побачення!\n"; break;
        default: cout << "Невірний вибір.\n";
        }
    } while (choice != 0);
    return 0;
}