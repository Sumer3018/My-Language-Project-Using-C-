blueprint Test {
    define get_name(first) {
        yield first;
    }
}
instance Test t;
lets_print {t.get_name("Alice")};