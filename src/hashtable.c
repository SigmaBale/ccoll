/*
 * This is implementation of Hash Table (hash map).
 * Hash table is an abstract data type that maps
 * keys to values.
 * Main properties of hash table are O(1) lookup,
 * insert and delete time complexity.
 *
 * General implementation of hash table is something
 * like this:
 *  1. Use a hash function to generate deterministic
 *      hash from the given value.
 *  2. Use that hashvalue to index into the array.
 *  3. Array will consist of pointers to association lists or
 *      dynamically sized data structure (to cope with collisions)
 *      which hold the value/s.
 *
 *  Note:
 *  Array (Hash Table) can hold pointers (or hold
 *  structures directly) to any arbitrary
 *  structure that might hold arbitrary number of
 *  members depending on the need, but generally
 *  it will mimic structure of association list.
 *  Association list holds a key, value and
 *  next element in the association list.
 *
 * So knowing the general idea of Hash table we can
 * deduce that having effiecient hash function would
 * lead to having the most performant time complexity.
 *
 * Lots of collisions usually result in a bad hashing
 * function implementation.
 * There are plenty of good hashing functions available
 * in many popular open source projects, of course
 * copy pasting the hash function and shipping it
 * is not generally a great idea.
 * We still want this to be as generic as possible.
 * Also hash function performance might perform
 * differently on certain kinds of inputs.
 *
 * For this reason when constructing the HashTable
 * we will give the user of this library obligation
 * to provide his own hash function and comparison function
 * for comparing values inside the association list.
 *
 * This implementation of hash function is subject to
 * change, main goal of this source file is to start
 * with a basic implementation that is reliable and works.
 * Then expand on it implementing generic behaviour,
 * better hashing methods/optimizations, and so on.
 */

struct HashTable { };
