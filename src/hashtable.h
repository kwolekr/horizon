void __fastcall InsertValue(char *key, void *newentry, LPCHAIN *table, int tablelen);
void __fastcall RemoveValue(char *key, LPCHAIN *table, int tablelen);
void *__fastcall GetValue(char *key, LPCHAIN *table, int tablelen);
void __fastcall ResetTableContents(LPCHAIN *table, int tablelen);
unsigned int __fastcall hash(unsigned char *key, int arraylen);

