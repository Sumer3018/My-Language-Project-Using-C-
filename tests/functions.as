define factorial(n) {
    if (n <= 1) {
        yield 1;
    }
    yield n + factorial(n - 1);
}

// Usage
let result := factorial(5);
lets_print{result};