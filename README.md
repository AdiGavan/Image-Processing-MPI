"Copyright [2018] Gavan Adrian-George, 334CA"

Nume, prenume: Gavan, Adrian-George

Grupa, seria: 334CA

Tema 3 APD - Image processing with MPI

Prezentarea implementarii:
==========================

A. Logica implementarii:
========================

- Procesul master (ultimul proces) formeaza un vector cu filtrele primite ca parametru.
- Acesta trimite numele filtrelor tuturor proceselor cu broadcast.
- Procesul master citeste imaginea de intrare.
- Pentru a spori eficienta, fiecare proces va avea de aplicat toate filtrele asupra
unui numar de linii din matricea initiala. Procesul master va trimite fiecarui proces
un interval de linii din matricea initiala. Si procesul master aplica filtrele asupra
unui numar de linii.
- Se pune o bariera inainte de inceperea aplicarii filtrelor pentru a ne asigura ca
toate procesele au primit datele si au initializat resursele de care au nevoie.
- In acest program trebuie ca procesele sa comunice intre ele. Mai specific, pentru
a aplica un filtru pe un pixel ai nevoie de vecinii pixelului. Daca avem 3 procese,
procesul din mijloc va avea nevoie pentru prima linie pe care o proceseaza de ultima
linie a primului proces si de prima linie a celui de-al treilea proces. Primul proces
va avea si el nevoie de prima linie a procesului 2 si procesul 3 va avea nevoie de
ultima linie a procesului 2. Cu alte cuvinte, un proces trebuie sa poata schimba o
linie cu procesul anterior lui si procesul de dupa el.
- Procesul master se uita daca este un singur proces care ruleaza sau mai multe. Daca
sunt mai multe procese care ruleaza si procesul master are linii de procesat, primeste
si transmite cate o linie cu procesul anterior lui (daca este master => ultimul proces
=> nu are alt proces dupa el, ci doar inainte).
- Procesele care nu sunt master (daca au linii de procesat) transmit si primesc cate
o linie de la procesele vecine.
- Se calculeaza matricea unui filtru in functie de filtrul curent.
- Pentru fiecare pixel in parte se aplica filtrul curent. Fiecare proces trece prin
fiecare pixel al liniilor care i-au fost date si aplica filtrul. Se tine cont si de
faptul ca unii pixeli fac parte din marginile imaginii.
- Se copiaza pixelii noi in matricea initiala a fiecarui proces pentru a putea aplica
urmatorul filtru.
- Se pune o bariera pentru a se asigura faptul ca toate procesele au aplicat filtrul
curent.
- La urmatorul filtru se reiau pasii anteriori, incepand cu comunicarea dintre procese.
- Dupa ce fiecare proces a aplicat toate filtrele, procesul master primeste toate
liniile prelucrate de la celelalte procese pentru a asambla matricea finala de pixeli.
- Se scrie noua imagine in fisierul de output.

B. Structuri si functii folosite:
==================================

a. elements - Structura pentru a retine startul pt fiecare proces si numarul de linii

b. rgb - Structura pentru a retine fiecare culoare a pixelului.

c. image - Structura pentru a retine datele unei imagini.

1. readInput:
- Retine mai intai tipul imaginii, lungimea, latimea si valoarea maxima.
- Daca este imagine alb-negru, se aloca memorie variabilei img->pixels_bw,
matrice ce va contine valorile pixelilor din matricea de intrare.
- Daca este imagine color, se aloca memorie variabilei img->pixels, matrice
ce va contine valorile pixelilor din matricea de intrare (fiecare element al
matricei este o structura cu 3 variabile, pentru a retine cele 3 culori ale
unui pixel).

2. free_memory_BW si free_memory_Color:
- Functii care elibereaza memoria in functie de tipul imaginii.

3. writeData:
- Deschide fisierul de iesire si scrie mai intai tipul imaginii, latimea,
lungimea si valoarea maxima.
- Daca imaginea este alb-negru apeleaza functia writeDataBW.
- Daca imaginea este color apeleaza functia writeDataColor.
- Dupa ce datele sunt scrise in fisier, se apeleaza una din functiile de
eliberare a memoriei (pentru imaginea de intrare).

4. writeDataBw si writeDataColor:
- Scriu fiecare linie din matricea pixelilor imaginii de iesire in fisierul de
iesire.

5. assign_filter_values:
- In functie de tipul filtrului, se atribuie valorile corespunzatoare fiecarei pozitii
din matricea filtrelor.

C. Logica proceselor mai detaliata:
===================================

1. Procesul master

- Formeaza vectorul de filtre si il trimite tuturor proceselor.
- Apeleaza functia readInput si citeste imaginea de intrare.
- Pentru fiecare proces in parte calculeaza inceputul si finalul intervalului de linii
corespunzator, retine pozitia de inceput a intervalului si numarul de linii pe care il
va trimite unui proces.
- Trimite liniile proceselor, iar daca unul din procese va fi ultimul proces care va
primi linii de procesat, ii trimite tag-ul 11 sau 10 (in functie de imagine color sau nu).
- Se aloca memorie pentru matricea in care se retin temporar valorile pixelilor dupa
aplicarea unui filtru.
- Se trece prin fiecare filtru ce trebuie aplicat.
- Se trimite prima linie ce trebuie calculata de master la procesul anterior si se
primeste ultima linie a procesului anterior.
- Se calculeaza valorile matricei filtrului corespunzator.
- Se trece prin fiecare pixel si se testeaza daca face parte din marginea imaginii sau nu.
Daca face parte => ia valoare inititala, daca nu face parte => se aplica filtrul conform
cerintei. Daca imaginea este color se aplica filtrul pe fiecare culoare a pixelului in
parte.
- Valorile noi calculate ale pixelilor sunt copiati in matricea initiala pentru a putea
fi folosite la urmatorul filtru. In cazul procesului master, matricea initiala si finala
va fi matricea din "input", pentru ca atunci cand se va face asamblarea datelor toate
liniile vor fi suprascrise cu valorile corect calculate.
- Dupa aplicarea filtrelor, se primesc pe rand fiecare linie de la fiecare proces si
se suprascrie linia corespunzatoare din "input".
- Se apeleaza functia writeData pentru a forma fisierul de output.

2. Procesele care nu sunt master

- Primesc filtrele ce trebuie aplicate.
- Primesc numarul de linii pe care il au de procesat si numarul de pixeli de pe o linie.
- Trebuie sa se aloce spatiul necesar pentru matricea initiala + vecinii pe care ii va
primi procesul. Daca procesul este primul proces sau ultimul care are de procesat linii,
atunci matricea va avea dimensiunea numarului de linii primit de la procesul master + 1,
pentru ca va primi doar o linie de la vecini. Altfel, daca are linii de procesat va avea
dimensiunea egala cu numarul de linii primit de la master + 2 (ca sa aiba loc pentru cele
2 linii pe care le va primi de la vecini).
- Se pune bariera pentru a se asigura ca toate procesele si-au alocat resursele necesare.
- Se trece prin fiecare filtru.
- Mai intai se realizeaza comunicarea cu vecinii. Se trimit linii la procesele vecine si
se primesc linii de la procesele vecine. Se are in calcul si cazul cand procesul curent
este primul sau ultimul cu linii de procesat (caz in care se comunica doar cu un vecin).
- Se calculeaza valorile matricei filtrului corespunzator.
- Se trece prin fiecare pixel si se testeaza daca face parte din marginea imaginii sau nu.
Daca face parte => ia valoare inititala, daca nu face parte => se aplica filtrul conform
cerintei. Daca imaginea este color se aplica filtrul pe fiecare culoare a pixelului in
parte.
- Cand se parcurg pixelii, se are in vedere sa nu se parcurga prima sau ultima linie acolo
unde este cazul. Pentru acest lucru se folosesc variabilele "offset_start" (pentru prima
linie) si "offset_end" (pentru ultima linie).
- Valorile noi calculate ale pixelilor sunt copiati in matricea initiala locala pentru a
putea fi folosite la urmatorul filtru. Si aici trebuie avut grija ca liniile sa se copieze
in pozitia corespunzatoare, pentru a nu fi suprascrise de liniile primite de la vecini.
- Dupa ce s-au aplicat toate filtrele, se trimit toate liniile catre procesul master
pentru asamblare.
