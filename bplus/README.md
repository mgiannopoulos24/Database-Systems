# Εργασία Συστημάτων Βάσεων Δεδομένων - B+ Tree Implementation

Η εργασία αυτή στοχεύει στην υλοποίηση ενός B+ δέντρου χρησιμοποιώντας διαχείριση σε επίπεδο block. Η υλοποίηση επικεντρώνεται στη δημιουργία και διαχείριση ενός B+ δέντρου για αποτελεσματική αποθήκευση και ανάκτηση εγγραφών.

## Υλοποιημένα Τμήματα

### Βασική Δομή B+ Δέντρου
Η βασική δομή του B+ δέντρου ορίζεται στο αρχείο `bp_file.h`:

```c
typedef struct {
    int root; // Δείκτης στη ρίζα του δέντρου
    int height; // Ύψος του δέντρου
} BPLUS_INFO;
```

### Συναρτήσεις B+ Δέντρου
Οι λειτουργίες για το B+ δέντρο υλοποιούνται στα `bp_file.c` και `bp_file.h`. Οι κύριες λειτουργίες περιλαμβάνουν:

1. **BP_CreateFile** - Δημιουργεί και αρχικοποιεί ένα άδειο B+ δέντρο:
```c
int BP_CreateFile(char *fileName) {
    CALL_BF(BF_CreateFile(fileName));
    int file_desc;
    CALL_BF(BF_OpenFile(fileName, &file_desc));

    BF_Block *block;
    BF_Block_Init(&block);
    CALL_BF(BF_AllocateBlock(file_desc, block));
    
    char *data = BF_Block_GetData(block);
    memset(data, 0, BF_BLOCK_SIZE); // Αρχικοποίηση του block με μηδενικά

    // Αποθήκευση μεταδεδομένων B+ δέντρου στο πρώτο μπλοκ
    BPLUS_INFO info;
    info.root = -1; // Ρίζα δεν έχει δημιουργηθεί ακόμα
    info.height = 0;
    memcpy(data, &info, sizeof(BPLUS_INFO));

    BF_Block_SetDirty(block);
    CALL_BF(BF_UnpinBlock(block));
    BF_Block_Destroy(&block);
    CALL_BF(BF_CloseFile(file_desc));

    return 0;
}
```

2. **BP_OpenFile** - Ανοίγει ένα υπάρχον B+ δέντρο:
```c
BPLUS_INFO* BP_OpenFile(char *fileName, int *file_desc) {
    if (BF_OpenFile(fileName, file_desc) != BF_OK) {
        return NULL;
    }

    BF_Block *block;
    BF_Block_Init(&block);
    
    if (BF_GetBlock(*file_desc, 0, block) != BF_OK) {
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }
    
    char *data = BF_Block_GetData(block);
    BPLUS_INFO *info = malloc(sizeof(BPLUS_INFO));
    if (info == NULL) {
        BF_UnpinBlock(block);
        BF_Block_Destroy(&block);
        BF_CloseFile(*file_desc);
        return NULL;
    }
    memcpy(info, data, sizeof(BPLUS_INFO));
    
    BF_UnpinBlock(block);
    BF_Block_Destroy(&block);
    return info;
}
```

3. **BP_CloseFile** - Κλείνει το B+ δέντρο:
```c
int BP_CloseFile(int file_desc, BPLUS_INFO* info) {
    if (info) {
        free(info); // Απελευθέρωση της δομής μεταδεδομένων
    }
    return BF_CloseFile(file_desc) == BF_OK ? 0 : -1;
}
```

### Λειτουργίες Κόμβων Δεδομένων
Οι λειτουργίες για τους κόμβους δεδομένων υλοποιούνται στο `bp_datanode.c`:

1. **insert_into_leaf** - Εισαγωγή εγγραφής σε φύλλο:
```c
bool insert_into_leaf(BPLUS_DATA_NODE *leaf, Record record) {
    if (leaf->key_count >= MAX_KEYS) {
        return false; // Ο κόμβος είναι γεμάτος
    }
    // Εισαγωγή της εγγραφής με ταξινόμηση κατά id
    int i;
    for (i = leaf->key_count - 1; i >= 0 && leaf->keys[i] > record.id; i--) {
        leaf->keys[i + 1] = leaf->keys[i];
        leaf->records[i + 1] = leaf->records[i];
    }
    leaf->keys[i + 1] = record.id;
    leaf->records[i + 1] = record;
    leaf->key_count++;
    return true;
}
```

2. **find_in_leaf** - Αναζήτηση εγγραφής σε φύλλο:
```c:bplus/src/bp_datanode.c
Record* find_in_leaf(BPLUS_DATA_NODE *leaf, int id) {
    for (int i = 0; i < leaf->key_count; i++) {
        if (leaf->keys[i] == id) {
            return &leaf->records[i];
        }
    }
    return NULL; // Δεν βρέθηκε
}
```

## Εγκατάσταση και Χρήση

### Δημιουργία Εκτελέσιμου
```bash
make bp
```

### Εκτέλεση
```bash
./build/bp_main
```

## Τεχνικές Λεπτομέρειες

- Block size: 512 bytes
- Μέγιστος αριθμός ανοιχτών αρχείων: 100
- Μέγιστος αριθμός blocks στη μνήμη: 100
- Υποστηριζόμενοι αλγόριθμοι αντικατάστασης: LRU, MRU

## Σημαντικές Σημειώσεις

1. Όλα τα blocks πρέπει να γίνονται unpin μετά τη χρήση τους
2. Σε περίπτωση αλλαγών στα δεδομένα, το block πρέπει να σημειώνεται ως dirty
3. Η μνήμη που δεσμεύεται δυναμικά πρέπει να απελευθερώνεται κατάλληλα

## Περιορισμοί

- Μέγιστος αριθμός κλειδιών ανά κόμβο: 4 (MAX_KEYS)
- Το δέντρο υποστηρίζει μόνο μοναδικά κλειδιά
- Δεν υποστηρίζεται διαγραφή εγγραφών

## Συντελεστές
- **Δημήτρης Σκόνδρας-Μέξης, AM 1115202200161**
- **Ελένη Μεταλλίδου, AM 1115202200272**
- **Μάριος Γιαννόπουλος, AM 1115202000032**

## Σκοπός της Εργασίας
Η εργασία αυτή είναι μέρος ενός ευρύτερου σκοπού να κατανοηθεί η εσωτερική λειτουργία των Συστημάτων Διαχείρισης Βάσεων Δεδομένων (ΣΔΒΔ) στη διαχείριση εγγραφών και blocks. Η υλοποίηση βασίζεται σε μια υπάρχουσα βιβλιοθήκη διαχείρισης σε επίπεδο block (BF), που προσομοιώνει τη λειτουργία cache ανάμεσα στον δίσκο και τη μνήμη. Μέσω της εργασίας αυτής, οι χρήστες αποκτούν εμπειρία στη χρήση εσωτερικών δομών δεδομένων και αλγορίθμων που βελτιώνουν την απόδοση των ΣΔΒΔ, ειδικότερα μέσω της υλοποίησης και χρήσης B+ δέντρων.
