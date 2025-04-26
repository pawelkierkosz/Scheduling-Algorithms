#include <cstdlib>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <ctime>
#include <cmath>
#include <random>
#include <cstring>
#include <iomanip>


using namespace std;

struct foo {
    int numTasks;
    int maxProcs;
    int maxJobs;
};


struct Task {
    int id;
    int gotowosc_r;
    int czas_trwania_p;
    int rozmiar;
};

// Funkcja do odczytywania pliku SWF - zwraca ilosc realnie odczytanych zadan (pomija te o zerowym czasie trwania)

foo odczyt_pliku(const char* filename, Task tasks[], int maxTasks) {

    ifstream file(filename);

    // Sprawdzenie poprawnosci otwarcia pliku

    if (!file) {
        cerr << "Nie mozna otworzyc pliku " << filename << endl;
        exit(EXIT_FAILURE);
    }

    char line[10000];
    int numTasks = 0;
    int buf;

    foo output;

    // Poczatek odczytu

    while (file.getline(line, sizeof(line))) {

        if (strlen(line) == 0) continue;
        if (line[0]==';') {
            sscanf(line, "; MaxProcs: %d", &output.maxProcs);
            sscanf(line, "; MaxJobs: %d", &output.maxJobs);
        }
        else{
            int spraw;
            int czy_wczytal = sscanf(line, "%d", &spraw);
            if (czy_wczytal!=1) continue;
            Task task;
            sscanf(line, "%d %d %d %d %d", &task.id, &task.gotowosc_r, &buf, &task.czas_trwania_p, &task.rozmiar);

            if (task.czas_trwania_p <= 0 || task.gotowosc_r < 0 || task.rozmiar <= 0) continue;
            tasks[numTasks++] = task;
        }
        if (numTasks >= maxTasks) break;
    }
    if (numTasks < output.maxJobs) {
        output.numTasks = numTasks;
    }
    else {
        output.numTasks = output.maxJobs;
    }

    file.close();
    return output;
}


void punkt_zaczepienia(vector<int> &brzeg, vector<int> &pkt_zacz) {
    int x = brzeg[0];
    int y = 0;

    for (int i = 1; i < brzeg.size(); ++i) {
        if (brzeg[i] < x) {
            x = brzeg[i];
            y = i;
        }
    }
    pkt_zacz = {x, y};
}

int wyznacz_wys_krawedzi_w_pz(vector<int> brzeg, vector<int> pkt_zacz, int m) {
    int x = pkt_zacz[0];
    int y = pkt_zacz[1];
    int wys_kraw = 0;

    while (y + wys_kraw < m && brzeg[y + wys_kraw] == x) {
        wys_kraw++;
    }

    return wys_kraw;
}

vector<int> wyznacz_zadania_w_pz(vector<int> zad_do_przydzielenia, vector<int> pkt_zacz, int krawedz, vector<int> r, vector<int> s) {
    vector<int> zadania_w_pz;
    int x = pkt_zacz[0];
    int ile = zad_do_przydzielenia.size();

    for (int i = 0; i < ile; ++i) {
        int idx_zad = zad_do_przydzielenia[i];
        if (x >= r[idx_zad] && s[idx_zad] <= krawedz) {
            zadania_w_pz.push_back(idx_zad);
        }
    }

    return zadania_w_pz;
}


void zmien_brzeg(int ktore_zadanie, vector<int> pkt_zacz, vector<int> p, vector<int> s, vector<int> &brzeg) {
    int y = pkt_zacz[1];
    for (int i = 0; i < s[ktore_zadanie]; ++i) {
        brzeg[y] += p[ktore_zadanie];
        y++;
    }
}

void wstaw_przestoj(vector<int> zad_do_przydzielenia, vector<int> pkt_zacz, int kraw, vector<int> r, vector<int> &brzeg, int m) {
    int x = pkt_zacz[0];
    int y = pkt_zacz[1];

    vector<int> czasy_roz;

    for (int zad : zad_do_przydzielenia) {
         czasy_roz.push_back(r[zad]);
    }

    int start_nast_zad = *min_element(czasy_roz.begin(), czasy_roz.end());

    if (x < start_nast_zad) {
        for (int i = 0; i < brzeg.size(); ++i) {
            if (brzeg[i] < start_nast_zad) {
                brzeg[i] = start_nast_zad;
            }
        }
    } else {
        int dl_przestoju;
        if (y - 1 < 0) {
            dl_przestoju = brzeg[y + kraw];
        } else if (y + kraw >= m) {
            dl_przestoju = brzeg[y - 1];
        } else {
            dl_przestoju = min(brzeg[y - 1], brzeg[y + kraw]);
        }

        for (int i = 0; i < kraw; ++i) {
            brzeg[y] = dl_przestoju;
            ++y;
        }
    }
}

void zapis_do_pliku(ofstream& wynik, int ktore_zadanie, vector<int> pkt_zacz, vector<int> ind_zad, vector<int> p, vector<int> s) {
    vector<int> linia = {ind_zad[ktore_zadanie], pkt_zacz[0], pkt_zacz[0] + p[ktore_zadanie]};
    int poczatek = pkt_zacz[1] + 1;
    int koniec = pkt_zacz[1] + s[ktore_zadanie];
    for (int i = poczatek; i <= koniec; ++i) {
        linia.push_back(i);
    }

    for (int i = 0; i < linia.size(); ++i) {
        wynik << linia[i];
        if (i < linia.size() - 1) {
            wynik << " ";
        }
    }
    wynik << endl;
}


vector<int> generuj_permutacje(int n){ // generowanie losowej permutacji
    vector<int> permutacja(n);
    for (int i = 0; i < n; ++i) {
        permutacja[i] = i;
    }
    random_shuffle(permutacja.begin(), permutacja.end());

    return permutacja;
}

vector<int> generuj_permutacje2(int n, vector<int> r){ // generowanie permutacji po czasach gotowosci

    vector<int> permutacja(n);
    for (int i = 0; i < n; ++i) {
        permutacja[i] = i;
    }
    sort(permutacja.begin(), permutacja.end(), [&r](int a, int b) {
        return r[a] < r[b];
    });
    return permutacja;
}



long long dekoduj_i_ocen(chrono::time_point<chrono::high_resolution_clock> start, chrono::seconds limit,
                   const char* plik, vector<int> permutacja, int m, int n,
                   vector<int> ind_zad, vector<int> r, vector<int> p, vector<int> s, bool zapis){

    vector<int> brzeg(m, 0);
    vector<int> pkt_zacz(2,0);
    vector<int> zad_do_przydzielenia = permutacja;
    vector<int> zad_przydzielone = {};
    long long Csum = 0;
    ofstream wynik;
    if (zapis) wynik.open(plik);

    while (zad_do_przydzielenia.size() > 0) {
        punkt_zaczepienia(brzeg, pkt_zacz);
        int krawedz = wyznacz_wys_krawedzi_w_pz(brzeg, pkt_zacz, m);
        vector<int> zadania_w_pz = wyznacz_zadania_w_pz(zad_do_przydzielenia, pkt_zacz, krawedz, r, s);


        if (zadania_w_pz.size() > 0) {
            int ktore_zadanie = zadania_w_pz[0];
            Csum = Csum + pkt_zacz[0] + p[ktore_zadanie];
            if (zapis) zapis_do_pliku(wynik, ktore_zadanie, pkt_zacz, ind_zad, p, s);

            zad_przydzielone.push_back(ktore_zadanie);
            zad_do_przydzielenia.erase(remove(zad_do_przydzielenia.begin(), zad_do_przydzielenia.end(), ktore_zadanie), zad_do_przydzielenia.end());

            zmien_brzeg(ktore_zadanie, pkt_zacz, p, s, brzeg);
         }
        else {
            wstaw_przestoj(zad_do_przydzielenia, pkt_zacz, krawedz, r, brzeg, m);
         }
        auto pomCzasu = chrono::high_resolution_clock::now();
        auto dlCzasu = chrono::duration_cast<chrono::seconds>(pomCzasu - start);
        //cout << dlCzasu.count()<<endl;
        if (dlCzasu >= limit) return -1;
     }

    //int Cmax = *max_element(brzeg.begin(), brzeg.end());  // gdy optymalizacja bedzie po Cmax
    if (zapis) wynik.close();
    //return Cmax;  // gdy optymalizacja bedzie po Cmax
    return Csum;    // gdy optymalizacja bedzie po Csum
}

vector<int> modyfikuj_perm1(vector<int> permut, int n){   //zamiana dowolnych elementow w permutacji
    int pkt_zamiany1;
    int pkt_zamiany2;

    pkt_zamiany1 = rand() % n;
    pkt_zamiany2 = pkt_zamiany1;
    while (pkt_zamiany2 == pkt_zamiany1) {
        pkt_zamiany2 = rand() % n;
    }
    swap(permut[pkt_zamiany1], permut[pkt_zamiany2]);
    return permut;
}

vector<int> modyfikuj_perm2(vector<int> permut, int n){ //zamiana sasiadujacych elementow w permutacji
    int pkt_zamiany;
    pkt_zamiany = rand() % (n-1);

    swap(permut[pkt_zamiany], permut[pkt_zamiany+1]);
    return permut;
}


int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Uzycie: %s <plik_wejsciowy> <plik_wyjsciowy> <ilosc_zadan_do_wczytania>\n",argv[0]);
        exit(EXIT_FAILURE);
    }

    const char* inputFilename = argv[1];
    const char* outputFilename = argv[2];
    int maxTasks = atoi(argv[3]);

    int maxCzas = 300; //maksymalny czas obliczen w sek
    double Tk = 0.0001; //temperatura koncowa
    int maxBezPop = 400; // //maksymalna liczba iteracji bez poprawy

    srand(time(NULL));
    Task tasks[maxTasks];

    foo output = odczyt_pliku(inputFilename, tasks, maxTasks);

    int maxProcs = output.maxProcs;
    int numTasks = output.numTasks;
    int maxJobs = output.maxJobs;

    int m = maxProcs;  // liczba procesorow
    int n = numTasks;   // liczba zadan

    cout << "maxProcs: "<< maxProcs << endl;
    cout << "numTasks: " << numTasks << endl;
    cout << "maxJobs: " << maxJobs << endl;

    vector<int> ind_zad;
    vector<int> r;
    vector<int> p;
    vector<int> s;


    for(int i = 0; i < numTasks; i++) {
        ind_zad.push_back(tasks[i].id);
        r.push_back(tasks[i].gotowosc_r);
        p.push_back(tasks[i].czas_trwania_p);
        s.push_back(tasks[i].rozmiar);
    }


    // Parametry algorytmu uzyskane w wyniku strojenia
    int liczIterWewn = 10; // liczba iteracji wewnetrznych
    double Tp = 10; // temperatura poczatkowa
    double alfa = 0.99; // wspolczynnik schladzania
    // Koniec parametrow

    double T = Tp;
    int glownaPetla = 1;
    long long Csum;
    long long minCsum;
    long long noweCsum = 0;
    int delta;
    int bez_poprawy = 1;

    vector<int> najlepszy(n);

    auto startZegara = chrono::high_resolution_clock::now();

    //vector<int> permutacja = generuj_permutacje(n); // losowa permutacja
    vector<int> permutacja = generuj_permutacje2(n, r); // permutacja po czasach gotowosci

    Csum = dekoduj_i_ocen(startZegara, chrono::seconds(maxCzas), outputFilename, permutacja, m, n, ind_zad, r, p, s, 0);
    if (Csum == -1) {
        cout << "\nW limicie czasu nie udalo sie wygenerowac zadnego rozwiazania.\n";
    }
    else {
        minCsum = Csum;
        najlepszy = permutacja;

        while (T > Tk && bez_poprawy < maxBezPop && noweCsum != -1 && maxTasks != 1){    //   Glowna petla

            for (int wewnetrzna = 0; wewnetrzna < liczIterWewn; wewnetrzna++){   //   Wewnetrzna petla
                vector<int> permut_nowa = modyfikuj_perm1(permutacja, n); // zmien permutacje
                noweCsum = dekoduj_i_ocen(startZegara, chrono::seconds(maxCzas), outputFilename, permut_nowa, m, n, ind_zad, r, p, s, 0);
                if (noweCsum == -1){
                    cout << "\n\nPrzekroczono limit czasu.\n";
                    break;
                }
                if (noweCsum < minCsum){
                    minCsum = noweCsum;
                    najlepszy = permut_nowa;
                    bez_poprawy = 1;
                }

                delta = noweCsum - Csum;
                if (delta < 0) {
                    permutacja = permut_nowa;
                    Csum = noweCsum;
                }
                else {
                    bez_poprawy++;
                    if (bez_poprawy >= maxBezPop) break;
                    int losowaInt = rand();
                    double losowaReal = static_cast<double>(losowaInt) / RAND_MAX;
                    double prawdopodob = exp(-delta / T);
                    if (losowaReal < prawdopodob){
                        permutacja = permut_nowa;
                        Csum = noweCsum;
                    }
                }
            }
            T = alfa*T;
            glownaPetla++;
            cout << "\r";
            cout << fixed << setprecision(4)<<"Aktualna temperatura: " << T ;
        }

        auto stop = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - startZegara);
        cout << endl<<endl;
        cout <<"Liczba wykonanych iteracji glownej petli: " << glownaPetla << endl;
        cout <<"Temperetura przy wyjsciu z glownej petli: " << T << endl;
        cout <<"Liczba iteracji bez poprawy: " << bez_poprawy << endl;
        printf("Czas obliczen  %.6f seconds.\n", elapsed.count()* 1e-9);

        minCsum = dekoduj_i_ocen(startZegara, chrono::seconds(2*maxCzas),outputFilename, najlepszy, m, n, ind_zad, r, p, s, 1); // dekodowanie i zapis do pliku

        cout <<endl << "SumCj = " << minCsum << endl;
        }
    return 0;
}

