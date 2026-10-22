/* One native hook, one sine table and one global DNII 16-slot allocation. */
extern void trxbd_init(void);
extern void trxsd_init(void);
extern void trxbd_process(void);
extern void trxsd_process(void);
void md_dual_init(void) { trxbd_init(); trxsd_init(); }
void md_dual_process(void) { trxbd_process(); trxsd_process(); }
