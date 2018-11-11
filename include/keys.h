#ifndef KEYS_H
#define KEYS_H

#ifndef CTRL
#define CTRL(x) ((x)&0x1f)
#endif // CTRL
// horizontal tab key
#ifndef KEY_HTAB
#define KEY_HTAB 9
#endif // KEY_HTAB
// login panel commands
#define DIALOG_PREV (KEY_MAX + 1)
#define DIALOG_NEXT (KEY_MAX + 2)
#define DIALOG_QUIT (KEY_MAX + 3)
#define DIALOG_ENTER (KEY_MAX + 4)

#endif // KEYS_H
