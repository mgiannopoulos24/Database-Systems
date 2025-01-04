# Εργασία Συστημάτων Βάσεων Δεδομένων - Εξωτερική Ταξινόμηση με Συγχώνευση

Η εργασία αυτή στοχεύει στην υλοποίηση του αλγορίθμου εξωτερικής ταξινόμησης με συγχώνευση (External Merge Sort) για τη διαχείριση και την ταξινόμηση μεγάλων αρχείων δεδομένων που δεν χωράνε στη μνήμη. Η υλοποίηση βασίζεται στις βιβλιοθήκες που αναπτύχθηκαν στις προηγούμενες εργασίες και χρησιμοποιεί το μοντέλο γλωσσικής επεξεργασίας ChatGPT για τη δημιουργία κώδικα και την αλληλεπίδραση με τον φοιτητή.

Μπορείτε να βρείτε την αλληλεπίδραση μας με το ChatGPT [εδώ](https://chatgpt.com/share/5326e67d-53ca-4031-b6df-17756208a53d).

## Υλοποιημένα Τμήματα
### Βιβλιοθήκη HP (Heap File)
Η βιβλιοθήκη HP περιλαμβάνει συναρτήσεις για τη διαχείριση αρχείων σωρού, όπως η δημιουργία, άνοιγμα, κλείσιμο, εισαγωγή και ανάκτηση εγγραφών. Οι κύριες συναρτήσεις είναι:

1. **HP_CreateFile** - Δημιουργεί ένα άδειο αρχείο σωρού.
2. **HP_OpenFile** - Ανοίγει ένα υπάρχον αρχείο σωρού.
3. **HP_CloseFile** - Κλείνει ένα αρχείο σωρού.
4. **HP_InsertEntry** - Εισάγει μια εγγραφή στο αρχείο σωρού.
5. **HP_GetRecord** - Ανακτά μια εγγραφή από το αρχείο σωρού.
6. **HP_UpdateRecord** - Ενημερώνει μια εγγραφή στο αρχείο σωρού.

### Βιβλιοθήκη SORT (Ταξινόμηση)
Η βιβλιοθήκη SORT περιλαμβάνει συναρτήσεις για την ταξινόμηση εγγραφών σε chunks (συρμούς) και την εφαρμογή του αλγορίθμου Merge Sort. Οι κύριες συναρτήσεις είναι:
1. **sort_FileInChunks** - Ταξινομεί ένα αρχείο σε chunks.
2. **sort_Chunk** - Ταξινομεί τις εγγραφές ενός chunk.
3. **mergeSort** - Υλοποιεί τον αλγόριθμο Merge Sort για την ταξινόμηση των εγγραφών.
4. **shouldSwap** - Καθορίζει αν δύο εγγραφές πρέπει να ανταλλάξουν θέσεις κατά τη διαδικασία ταξινόμησης.

### Βιβλιοθήκη MERGE (Συγχώνευση)
Η βιβλιοθήκη MERGE περιλαμβάνει συναρτήσεις για τη συγχώνευση ταξινομημένων chunks. Η κύρια συνάρτηση είναι:
1. **merge** - Συγχωνεύει ταξινομημένα chunks σε ένα νέο αρχείο.

### Βιβλιοθήκη CHUNK (Διαχείριση Chunks)

Η βιβλιοθήκη CHUNK περιλαμβάνει συναρτήσεις για τη διαχείριση και την επεξεργασία των chunks. Οι κύριες συναρτήσεις είναι:
1. **CHUNK_CreateIterator** - Δημιουργεί έναν iterator για την επανάληψη των chunks.
2. **CHUNK_GetNext** - Ανακτά το επόμενο chunk.
3. **CHUNK_GetIthRecordInChunk** - Ανακτά την i-οστή εγγραφή από ένα chunk.
4. **CHUNK_UpdateIthRecord** - Ενημερώνει την i-οστή εγγραφή σε ένα chunk.
5. **CHUNK_Print** - Εκτυπώνει τις εγγραφές ενός chunk.

## Εκτελέσιμα Αρχεία

Η εργασία περιλαμβάνει δύο κύρια εκτελέσιμα αρχεία:

1. `chunk_main.c`
Αυτό το εκτελέσιμο δημιουργεί ένα αρχείο σωρού, το γεμίζει με τυχαίες εγγραφές και στη συνέχεια χρησιμοποιεί έναν iterator για να διαβάσει και να εκτυπώσει τα chunks του αρχείου. Ο κώδικας αυτός δείχνει πώς μπορούν να χρησιμοποιηθούν οι συναρτήσεις της βιβλιοθήκης CHUNK για τη διαχείριση των chunks.

2. `sort_main.c`
Αυτό το εκτελέσιμο δείχνει πώς μπορεί να εφαρμοστεί ο αλγόριθμος εξωτερικής ταξινόμησης με συγχώνευση. Χρησιμοποιεί τις βιβλιοθήκες SORT και MERGE για να ταξινομήσει και να συγχωνεύσει τα δεδομένα σε ένα νέο αρχείο.

## Πώς να Χρησιμοποιήσετε τα Εκτελέσιμα

### Εκτελέσιμο `chunk_main`
Για τη δημιουργία και εκτέλεση του `chunk_main`:
```console
make chunk_main
./build/chunk_main
```

### Εκτελέσιμο `sort_main`
Για τη δημιουργία και εκτέλεση του `sort_main`:
```console
make sort_main
./build/sort_main
```

## Eνδεικτική Εκτέλεση
- `sort_main`:
```
File ./test1.db has 512 blocks
After the sort phase, file ./test1.db has 512 chunk(s)
After the merge phase 1, the output file ./test1.db1.db has 256 chunk(s)
After the merge phase 2, the output file ./test1.db2.db has 128 chunk(s)
After the merge phase 3, the output file ./test1.db3.db has 64 chunk(s)
After the merge phase 4, the output file ./test1.db4.db has 32 chunk(s)
After the merge phase 5, the output file ./test1.db5.db has 16 chunk(s)
After the merge phase 6, the output file ./test1.db6.db has 8 chunk(s)
After the merge phase 7, the output file ./test1.db7.db has 4 chunk(s)
After the merge phase 8, the output file ./test1.db8.db has 2 chunk(s)
After the merge phase 9, the output file ./test1.db9.db has 1 chunk(s)
File ./test2.db has 1280 blocks
After the sort phase, file ./test2.db has 256 chunk(s)
After the merge phase 1, the output file ./test2.db1.db has 64 chunk(s)
After the merge phase 2, the output file ./test2.db2.db has 16 chunk(s)
After the merge phase 3, the output file ./test2.db3.db has 4 chunk(s)
After the merge phase 4, the output file ./test2.db4.db has 1 chunk(s)
```

- `chunk_main`:
```console
Printing records in Chunk from Block 1 to Block 5:
0,Chrysa,Rezkalla,Kalamata
1,Nikos,Papadellis,Trikala
2,Pavlos,Katsaros,Mytilene
3,Aikaterini,Zachariou,Larissa
4,Apostolos,Oikonomou,Alexandroupoli
5,Kostas,Pappas,Rethymno
6,Ioannis,Trikalinos,Naxos
7,Efstathios,Zachariou,Kalymnos
8,Argyro,Papadakis,Kozani
9,Katerina,Zachariadis,Patras
10,Vasileios,Zachariadis,Heraklion
11,Achilleas,ParaskevopoulosIoannina,Ioannina
12,Violeta,Laskaris,Grevena
.
.
.
.
216,Vasileios,Gkikas,Chios
217,Nikola,Economou,Serres
218,Ioannis,Manolopoulos,Pyrgos
219,Aspasia,Koronis,Komotini
220,Violeta,Papadimitriou,Volos
221,Eirini,Dellis,Piraeus
222,Dora,Papadopoulos,Edessa
223,Olga,ParaskevopoulosKomotini,Komotini
224,Michalis,Nastou,Arta
End of Chunk Print
```

## Συντελεστές
- **Δημήτρης Σκόνδρας-Μέξης, AM 1115202200161**
- **Ελένη Μεταλλίδου, AM 1115202200272**
- **Μάριος Γιαννόπουλος, AM 1115202000032**

Τα μέλη αυτά συνεισέφεραν εξίσου στην υλοποίηση της εργασίας, αναπτύσσοντας τη λειτουργικότητα των βιβλιοθηκών και την εφαρμογή του αλγορίθμου εξωτερικής ταξινόμησης με συγχώνευση.

### Σκοπός της Εργασίας
Η εργασία αυτή είναι μέρος ενός ευρύτερου σκοπού να κατανοηθεί η εσωτερική λειτουργία των Συστημάτων Διαχείρισης Βάσεων Δεδομένων (ΣΔΒΔ) στη διαχείριση μεγάλων αρχείων δεδομένων που δεν χωράνε στη μνήμη. Η υλοποίηση βασίζεται σε μια υπάρχουσα βιβλιοθήκη διαχείρισης σε επίπεδο block (BF), που προσομοιώνει τη λειτουργία cache ανάμεσα στον δίσκο και τη μνήμη. Μέσω της εργασίας αυτής, οι χρήστες αποκτούν εμπειρία στη χρήση εσωτερικών δομών δεδομένων και αλγορίθμων που βελτιώνουν την απόδοση των ΣΔΒΔ, ειδικότερα μέσω της χρήσης εξωτερικής ταξινόμησης με συγχώνευση.