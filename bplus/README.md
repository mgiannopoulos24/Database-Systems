# Η εργασία έγινε από τους:
- **Δημήτρης Σκόνδρας-Μέξης, AM 1115202200161**
- **Ελένη Μεταλλίδου, AM 1115202200272**
- **Μάριος Γιαννόπουλος, AM 1115202000032**

Οι οποίοι συνέβαλαν εξίσου σε κάθε κομμάτι της άσκησης.

# Σχετικά με τα αρχεία:
- Δεν έχουν προστεθεί άλλα αρχεία
- Τα αρχεία που έχουν τροποποιηθεί είναι τα **bf_main.c, bp_file.h, bp_datanode.h, bp_datanode.c & bp_file.c**. Όλα τα άλλα μπορούν να αγνοηθούν.

# Πως εκτελούμε την άσκηση:
- Μέσω **make** δημιουργούμε το εκτελέσιμο αρχείο **bplus_main** που βρίσκεται εντός του καταλόγου **build/**.
- Εκτελούμε τρέχοντας: ./build/bplus_main
- Το αρχείο **data.db** διαγράφεται σε κάθε make

# Σχετικά με τους κώδικες
- Στο **bp_file.h** ουσιαστικά ορίζεται το struct για την πληροφορία του Β+ Δέντρου. Πρόκειται για τον κόμβο ρίζας αλλά και 
τον αριθμό block που βρίσκεται.
- Στο **bp_datanode.h** ορίζεται το struct που περιέχει την πληροφορία του κάθε κόμβου ενός Β+ Δέντρου και τις λειτουργίες του. 
  - Για τις πληροφορίες κάθε κόμβου έχουμε ότι:
    - Θεωρούμε ότι ο κάθε κόμβος έχει **MAX_KEYS κλειδιά** (ίσο με 4 όπως ορίζεται μέσω define) και **MAX_KEYS + 1** παιδιά. 
    - Κρατάμε την πληροφορία για το εάν είναι φύλλο ή όχι, όπως επίσης και ένα κόμβο στο επόμενο φύλλο. 
    - Επειδή δεν είναι δεδομένο ότι θα έχουμε σε κάθε κόμβο ακριβώς MAX_KEYS κλειδιά, ορίζουμε και την μεταβλητή **n** που μας δείχνει πόσα κλειδία έχουμε αυτήν την στιγμή.
    - Θεωρούμε ότι το δέντρο έχει μέχρι **MAX_NODES** παιδιά (ίσο με 32768 όπως ορίζεται μέσω define). Ο λόγος είναι για να αποφύγουμε την χρήση της malloc. Αν παρατηρήσουμε ότι το πλήθος των δεδομένων είναι μεγαλύτερο από MAX_NODES, μπορούμε απλά να το αυξήσουμε.
  - Για τις λειτουργίες του Β+ Δέντρου έχουμε ότι:
    - Περιέχει τις βασικές λειτουργίες όπως create, insert, search, delete & display.
- Στο **bp_datanode.c** υλοποιείται η διεπαφή του bp_datanode.h, μια σημείωση είναι ότι υπάρχουν βοηθητικές συναρτήσεις οι οποίες ορίζονται ως **static** και βοηθάνε αρκετά στην απλότητα του κώδικα.
- Στο **bp_file.c** υλοποιούμε την διεπαφή που αναφέρει η άσκηση. Χρησιμοποιεί την διεπαφή του **bp_datanode.h** και την **bf.h**.

# Ενδεικτική εκτέλεση:
```console
sdi2200161@linux14:~/Desktop/Database-Systems/bplus$ make && ./build/bplus_main
Compiling bplus_main ...
rm -f ./build/bplus_main data.db
gcc -g -I ./include -L./lib -Wl,-rpath=./lib ./examples/bp_main.c ./src/record.c ./src/bp_file.c ./src/bp_datanode.c ./src/bp_indexnode.c -lbf -o ./build/bplus_main -O2
Duplicate id detected: 336
Duplicate id detected: 379
Duplicate id detected: 784
Duplicate id detected: 528
Duplicate id detected: 500
Duplicate id detected: 292
Duplicate id detected: 100
Duplicate id detected: 231
Duplicate id detected: 338
Duplicate id detected: 857
Duplicate id detected: 127
Duplicate id detected: 179
Duplicate id detected: 211
Duplicate id detected: 379
Duplicate id detected: 573
Duplicate id detected: 159
Searching for: 159
(159,Sofia,Mavromatis,Ioannina)
Searching for: 161
(161,Dimitrios,Skondras,Piraeus)
Searching for: 1000
Cannot find an entry for id = 1000
```
- Τα μηνύματα για τα διπλότυπα εκτυπώνονται στο stderr.