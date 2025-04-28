blueprint Person {
    define greet() {
        lets_print{"Hello!"};
    }
}

// Instantiation
instance Person alice;
alice.greet();
