# MyOS GUI — wymagania i minimalny prototyp

## Cel dokumentu

Dokument opisuje minimalne wymagania dla pierwszego prototypu graficznego interfejsu MyOS. Zakres obejmuje główne ekrany systemu, podstawowe akcje użytkownika oraz spójny styl wizualny, który może zostać bezpośrednio odwzorowany w pierwszej wersji GUI.

## Główne ekrany GUI

### 1. Pulpit

Pulpit jest ekranem startowym systemu. Ma prezentować najważniejsze skróty, pasek statusu, aktywne okna oraz szybki dostęp do menu aplikacji.

Minimalna zawartość:

- tło systemowe z delikatnym gradientem,
- ikony skrótów do aplikacji,
- dolny lub górny panel statusu,
- obszar roboczy dla okien,
- przycisk otwierający menu aplikacji.

Minimalne akcje użytkownika:

- otwarcie menu aplikacji,
- uruchomienie aplikacji ze skrótu,
- przełączenie widocznego okna,
- zamknięcie aktywnego okna,
- sprawdzenie godziny i stanu systemu w panelu statusu.

### 2. Menu aplikacji

Menu aplikacji pozwala użytkownikowi przeglądać dostępne aplikacje i uruchamiać wybrane moduły systemu.

Minimalna zawartość:

- pole wyszukiwania aplikacji,
- lista podstawowych aplikacji,
- kategorie aplikacji,
- skróty do ustawień, menedżera plików i terminala.

Minimalne akcje użytkownika:

- otwarcie i zamknięcie menu,
- wyszukanie aplikacji po nazwie,
- uruchomienie ustawień,
- uruchomienie menedżera plików,
- uruchomienie terminala.

### 3. Ustawienia

Ekran ustawień służy do podstawowej konfiguracji wyglądu i zachowania systemu.

Minimalna zawartość:

- sekcja wyboru motywu jasnego lub ciemnego,
- sekcja akcentu kolorystycznego,
- podstawowe informacje o systemie,
- przełączniki przykładowych opcji, takich jak animacje lub kompaktowy układ.

Minimalne akcje użytkownika:

- przełączenie motywu jasnego i ciemnego,
- wybór koloru akcentu,
- włączenie lub wyłączenie przykładowej opcji,
- powrót do pulpitu lub innego aktywnego ekranu.

### 4. Menedżer plików

Menedżer plików umożliwia przeglądanie podstawowej struktury katalogów i plików w prototypie.

Minimalna zawartość:

- pasek ścieżki,
- lista folderów i plików,
- boczny panel szybkiego dostępu,
- podstawowe metadane zaznaczonego elementu.

Minimalne akcje użytkownika:

- otwarcie folderu,
- powrót do katalogu nadrzędnego,
- zaznaczenie pliku,
- utworzenie nowego folderu w widoku demonstracyjnym,
- odświeżenie listy plików.

### 5. Terminal

Terminal jest prostym ekranem tekstowym służącym do prezentacji komend systemowych w prototypie.

Minimalna zawartość:

- obszar historii komend,
- wiersz polecenia,
- przykładowe odpowiedzi systemu,
- informacja o aktualnym użytkowniku i ścieżce.

Minimalne akcje użytkownika:

- wpisanie komendy,
- zatwierdzenie komendy klawiszem Enter,
- wyczyszczenie historii komend,
- wyświetlenie przykładowej pomocy,
- zamknięcie terminala.

### 6. Panel statusu

Panel statusu prezentuje bieżący stan systemu i daje dostęp do szybkich ustawień.

Minimalna zawartość:

- aktualna godzina,
- status sieci,
- poziom baterii lub zasilania,
- wskaźnik motywu,
- przycisk szybkich ustawień.

Minimalne akcje użytkownika:

- podgląd godziny i statusu systemu,
- otwarcie szybkich ustawień,
- przełączenie motywu,
- otwarcie pełnych ustawień,
- zamknięcie panelu szybkich ustawień.

## Styl wizualny

### Motywy

GUI powinno obsługiwać dwa podstawowe motywy:

- **ciemny motyw** jako domyślny: ciemne tło, jasny tekst i wyraźne kolory akcentu,
- **jasny motyw**: jasne tło, ciemny tekst i ten sam system akcentów.

Przełączenie motywu powinno natychmiast aktualizować pulpit, okna, menu aplikacji i panel statusu.

### Kolory

Minimalna paleta kolorów:

- tło ciemne: `#0f172a`,
- powierzchnia ciemna: `#111827`,
- tło jasne: `#f8fafc`,
- powierzchnia jasna: `#ffffff`,
- tekst główny w ciemnym motywie: `#e5e7eb`,
- tekst główny w jasnym motywie: `#111827`,
- tekst drugorzędny: `#94a3b8`,
- akcent podstawowy: `#38bdf8`,
- akcent alternatywny: `#a78bfa`,
- kolor ostrzeżeń lub energii: `#f59e0b`.

### Układ

- Pulpit zajmuje całą dostępną przestrzeń ekranu.
- Panel statusu jest przyklejony do dolnej krawędzi ekranu.
- Okna aplikacji są wyśrodkowane lub wyrównane do siatki pulpitu.
- Menu aplikacji otwiera się jako panel nad pulpitem.
- Elementy powinny mieć spójne odstępy, zaokrąglenia i cienie.

### Ikony

- Ikony powinny być proste, czytelne i spójne wizualnie.
- W pierwszym prototypie dopuszczalne są ikony tekstowe lub emoji.
- Każda główna aplikacja powinna mieć rozpoznawalny symbol.

### Responsywność

- Interfejs powinien działać na szerokościach desktopowych i tabletowych.
- Na węższych ekranach okna powinny zajmować prawie całą szerokość widoku.
- Menu aplikacji powinno zmieniać układ listy na jedną kolumnę.
- Panel statusu powinien zachować dostęp do najważniejszych informacji nawet przy ograniczonej szerokości.

## Zakres pierwszego minimalnego prototypu

Pierwszy prototyp powinien być statyczną aplikacją webową uruchamianą z pliku `index.html`. Minimalna implementacja powinna zawierać:

- pulpit z ikonami aplikacji,
- otwierane menu aplikacji z wyszukiwarką,
- okno ustawień z przełącznikiem motywu,
- okno menedżera plików z przykładową listą plików,
- okno terminala z prostą obsługą komend demonstracyjnych,
- panel statusu z godziną, stanem sieci, baterią i szybkim przełącznikiem motywu.
