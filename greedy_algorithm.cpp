#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <chrono>
#include <cmath>
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
    int size;
};


foo odczyt_pliku(const char* filename, Task tasks[], int maxTasks) {

    ifstream file(filename);

    if (!file) {
        cerr << "Nie mozna otworzyc pliku " << filename << endl;
        exit(EXIT_FAILURE);
    }

    char line[10000];
    int numTasks = 0;
    int buf;

    foo output;

    while (file.getline(line, sizeof(line))) {

    if (strlen(line) == 0) continue;
	if (line[0]==';') {
            sscanf(line, "; MaxProcs: %d", &output.maxProcs);
            sscanf(line, "; MaxJobs: %d", &output.maxJobs);
	}
	else {
            int spraw;
            int czy_wczytal = sscanf(line, "%d", &spraw);
            if (czy_wczytal != 1) {
            	continue;
        }
            Task task;
            sscanf(line, "%d %d %d %d %d", &task.id, &task.gotowosc_r, &buf, &task.czas_trwania_p, &task.size);


            if (task.czas_trwania_p <= 0 || task.gotowosc_r < 0 || task.size <= 0) continue;
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

int wybor_zadania(vector<int> zadania_w_pz, vector<int> waga) {
    int naj_waga = waga[zadania_w_pz[0]];
    int najlepsze = zadania_w_pz[0]; 

    for (int i = 1; i < zadania_w_pz.size(); ++i) {
        int ktore = zadania_w_pz[i];

        if (waga[ktore] < naj_waga) {
            najlepsze = ktore;
            naj_waga = waga[ktore];
         }
    }
    return najlepsze;
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



int main(int argc, char *argv[]) {

    if (argc != 4) {
        fprintf(stderr, "Uzycie: %s <plik_wejsciowy> <plik_wyjsciowy> <ilosc_zadan_do_wczytania>\n",argv[0]);
        exit(EXIT_FAILURE);
    }


    const char* inputFilename = argv[1];
    const char* outputFilename = argv[2];
    int maxTasks = atoi(argv[3]);

    Task tasks[maxTasks];
    long long Csum = 0;           
    float Cavg;              

    foo output = odczyt_pliku(inputFilename, tasks, maxTasks);

    int maxProcs = output.maxProcs;
    int numTasks = output.numTasks;
    int maxJobs = output.maxJobs;

    vector<int> ind_zad;
    vector<int> r;
    vector<int> p;
    vector<int> s;

   for(int i = 0; i < numTasks; i++) {

        ind_zad.push_back(tasks[i].id);
        r.push_back(tasks[i].gotowosc_r);
        p.push_back(tasks[i].czas_trwania_p);
        s.push_back(tasks[i].size);
}
    ofstream wynik(argv[2]);

    cout << "maxProcs: " << maxProcs << endl;
    cout << "numTasks: " << numTasks << endl;
    cout << "maxJobs: " << maxJobs << endl;

    auto start = chrono::high_resolution_clock::now();


    int m = maxProcs;  // liczba procesorow
    int n = numTasks;   // liczba zadan

    vector<int> brzeg(m, 0);
    vector<int> pkt_zacz(2,0);
    vector<int> waga;
    vector<int> zad_przydzielone;
    vector<int> zad_do_przydzielenia;

    for (int i = 0; i < n; i++) {
        //waga.push_back(static_cast<float>(p[i]) * s[i]);
        waga.push_back(p[i]);
        zad_do_przydzielenia.push_back(i);
    }

    while (zad_do_przydzielenia.size() > 0) {

        auto end1 = chrono::high_resolution_clock::now();
        chrono::duration<double> duration1 = end1 - start;

        if (duration1.count() >  300) {
                cout << "\nPrzekroczono czas wykonywania\n";
                exit(1);
        }

        punkt_zaczepienia(brzeg, pkt_zacz);
        int krawedz = wyznacz_wys_krawedzi_w_pz(brzeg, pkt_zacz, m);
        vector<int> zadania_w_pz = wyznacz_zadania_w_pz(zad_do_przydzielenia, pkt_zacz, krawedz, r, s);

       if (zadania_w_pz.size() > 0) {

            int ktore_zadanie = wybor_zadania(zadania_w_pz, waga);
            Csum = Csum + pkt_zacz[0] + p[ktore_zadanie];                   
            zapis_do_pliku(wynik, ktore_zadanie, pkt_zacz, ind_zad, p, s);
            zad_przydzielone.push_back(ktore_zadanie);

            zad_do_przydzielenia.erase(remove(zad_do_przydzielenia.begin(), zad_do_przydzielenia.end(), ktore_zadanie), zad_do_przydzielenia.end());
            zmien_brzeg(ktore_zadanie, pkt_zacz, p, s, brzeg);
        }
	else {
            wstaw_przestoj(zad_do_przydzielenia, pkt_zacz, krawedz, r, brzeg, m);
        }
    }

    auto end = chrono::high_resolution_clock::now();
    auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - start);
    printf("Czas obliczen  %.6f seconds.\n", elapsed.count()* 1e-9);

    wynik.close();

    cout << endl;
    Cavg = static_cast<float>(Csum)/n;   
    int cmax = *max_element(brzeg.begin(), brzeg.end());
    cout << "Cmax = " << cmax << endl;
    cout << "SumCj = " << Csum << endl;   
    cout << fixed << setprecision(4) << "Cavg = " << Cavg << endl;   

    return 0;
}
