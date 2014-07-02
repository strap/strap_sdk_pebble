/*
Copyright 2014 EnSens, LLC D/B/A Strap

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

   http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef STRAP_H
#define STRAP_H

#define STRAP_FREQ_HIGH     1  // more data collection, but higher power drain
#define STRAP_FREQ_MED      2  // less data collection, with moderate power drain
#define STRAP_FREQ_LOW      3  // least data collection, lowest power drain

void strap_init();
void strap_deinit();
void strap_log_action(char *);
void strap_log_event(char *);
void strap_out_sent_handler(DictionaryIterator *, void *);
void strap_out_failed_handler(DictionaryIterator *, AppMessageResult , void *);
void strap_set_activity(char*);
void strap_set_freq(int);

#endif


