struct cursor;
struct container;

struct coord * cursor_pos_get(struct cursor *this);
struct cursor *cursor_new(struct container *co);
