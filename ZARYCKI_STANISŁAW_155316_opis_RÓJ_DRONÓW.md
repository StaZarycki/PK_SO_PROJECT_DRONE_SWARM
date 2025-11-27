## Link do repozytorium: <https://github.com/StaZarycki/PK_SO_PROJECT_DRONE_SWARM>

# Opis

Rój autonomicznych dronów liczy początkowo N egzemplarzy. Drony startują (i lądują) z ukrytej
platformy (bazy) na której w danym momencie może znajdować się co najwyżej P (P<N/2) dronów.
Dron, który chce wrócić do bazy, musi wlecieć przez jedno z dwóch istniejących wejść. Wejścia te są
bardzo wąskie, więc możliwy jest w nich jedynie ruch w jedną stronę w danej chwili czasu.
Zbyt długie przebywanie w bazie - ładowanie baterii, grozi jej przegrzaniem, dlatego każdy z dronów
opuszcza bazę po pewnym skończonym czasie T1i.

Jedno pełne ładowanie wystarcza na lot, który maksymalnie może trwać T2i (T2i = 2.5\*T1i). Przy
poziome naładowania baterii 20% dron automatycznie rozpoczyna powrót do bazy.
Jeżeli w trakcie lotu poziom naładowania baterii osiągnie 0%, dron ulega zniszczeniu.
Znajdujący się w bazie operator co pewien czas Tk stara się uzupełnić braki w liczbie dronów, pod
warunkiem, że w bazie jest wystarczająca ilość miejsca.
Dowódca systemu może dołożyć (sygnał1 do operatora) dodatkowe platformy startowe, które
pozwalają zwiększyć liczbę dronów maksymalnie do 2\*N egzemplarzy. Może również zdemontować
(sygnał2 do operatora) platformy startowe ograniczając bieżącą maksymalną liczbę egzemplarzy o
50%.

Dowódca systemu może do konkretnego drona (nawet tego, który jest w bazie w trakcie ładowania
baterii) wysłać polecenie wykonania ataku samobójczego (sygnał3). Jeżeli poziom naładowania
baterii jest niższy niż 20% dron ignoruje sygnał3.

Napisz program dowódcy sytemu, operatora i drona, tak by zasymulować cykl życia roju dronów.
Każdy z dronów jest utylizowany (wycofany z eksploatacji) po pewnym określonym czasie Xi, liczonym
ilością ładowań (pobytów w bazie). Raport z przebiegu symulacji zapisać w pliku (plikach) tekstowym.

# Testy

## Testy bazy

- Czy w bazie zawsze znajduje się co najwyżej N/2 dronów?
- Czy w danej chwili ruch w wejściu do bazy odbywa się tylko w jedną stronę?
- Czy drony nie siedzą w bazie zbyt długo (przegrzanie baterii)?

## Testy dronów

- Czy drony nie latają dłużej, niż 2.5 \* T1i (T2i)?
- Czy przy poziomie baterii poniżej 20% drony kierują się do bazy?
- Czy po tym, gdy bateria spada do 0%, dron jest niszczony (usuwany)?

## Testy operatora

- Czy operator co czas Tk uzupełnia braki w liczbie dronów?
- Czy operator nie uzupełnia dronów powyżej dopuszczalnej ilości?

## Testy dowódcy

- Czy sygnał3 powoduje zniszczenie drona?
    - Czy dron zignoruje sygnał3, jeśli ma poniżej 20% naładowania baterii?

## Testy platform

- Czy sygnał1 dodaje platformę?
- Czy sygnał2 odejmuje platformę?
- Czy każda platforma podwaja maksymalną ilość dronów?
