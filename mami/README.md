# Mami

Learning socket programming.

## Pipes and Filters pattern

```
+---------------------+ Pipeline +---------------------+
|                                                      |
|      ^     +----------------------------+     +      |
|      |     |    Application Protocol    |     |      |
|      |     +---^--------------------+---+     |      |
|      |         |                    |         |      |
|      |     +---+--------------------v---+     |      |
|      |     |          Session           |     |      |
|      |     +---^--------------------+---+     |      |
|      |         |                    |         |      |
|      |     +---+--------------------v---+     |      |
|      +     |    Transport (Socket/TCP)  |     v      |
|  Upstream  +----------------------------+ Downstream |
|   (Read)                                   (Write)   |
|                                                      |
+------------------------------------------------------+

     ^                         +----------------------+
     |     Pipe                |                      |
     +                         +----^------------+----+
                                    |            |
 +-------+                        +-+-+        +-v-+
 |       | Filter                 |XXX|        |   |
 +-------+                        |   |        |   |
                                  |   |        |XXX|
   +---+                          +-^-+        +-+-+
   |XXX|                            |            |
   |   | I/O Buffering Queue   +----+------------v----+
   |   |                       |                      |
   +---+                       +----------------------+
```
