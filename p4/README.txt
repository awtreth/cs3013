Author:				Mateus Amarante Araujo
Project ID:			Project 4
CS Class:			CS 3013 (Operating Systems) - WPI
Programming Language:		C
How to build the program:	make [all]

Checkpoint:

Implemented FIFO eviction algorithm
page_fault and evict auxiliar functions

Tried to minimize memory management extra resources

page_entry (4 bytes for now, but I'm only using 3)
Auxiliar variables: ssd_bitmap(16bytes), ram_bitmap(4bytes), ram_cursor(4bytes, could be 1), ssd_cursor(4bytes, could be 1) and ptable_cursor (4bytes, could be 2


Comments:
I've just realized the checkpoint was due Monday (not today, on Tuesday, as the last project). Sorry about that.

TODO:
-mutual exclusion
-other 2 eviction algorithms (Intend to do random and "second chance")
