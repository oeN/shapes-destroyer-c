typedef struct s_Component {
  int id;
} Component;

typedef struct {
  union {
    struct s_Component;
    Component c;
  };

  float x;
  float y;
} Vec2;

Vec2 v2 = {.x = 10, .y = 10, .c.id = 1};

typedef struct EntitiManager {
  Component components[];  // store the actual components
} EntitiManager;

typedef struct Entity {
  Component *components;  // array of pointers to components
} Entity;