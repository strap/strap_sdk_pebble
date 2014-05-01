#ifndef STRAP_H
#define STRAP_H

void strap_init();
void strap_deinit();
void strap_log_action(char *);
void strap_out_sent_handler(DictionaryIterator *, void *);
void strap_out_failed_handler(DictionaryIterator *, AppMessageResult , void *);
void strap_set_activity(char*);

#endif


