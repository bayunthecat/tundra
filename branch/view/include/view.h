#ifndef VIEW_H
#define VIEW_H

typedef struct View View;

View *makeView();

void run(View *engine);

void freeView(View *engine);

#endif // !VIEW_H
