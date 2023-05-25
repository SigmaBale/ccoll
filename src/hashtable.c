/*
 * This is implementation of hash table.
 * Hash table is an abstract data type that maps
 * keys to values.
 *
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
 *      or array might only hold pointers that directly point
 *      to the value if we chose to avoid collisions by open
 *      adressing.
 *
 * There is lot of nuance and possible optimizations and choices to
 * be made when creating a hash table from scratch.
 * This 'HashTable' will use open adressing using 'Robin Hood hashing' method.
 * It will use 'SipHash 2-4' that is hash flooding resistant (HashDoS).
 */
