namespace physics {
    namespace constants {
        int pi_scaled = 314; // We use int for now
        bool is_scientific = true;
    }

    char unit = 'm';

    int get_pi() {
        return physics::constants::pi_scaled;
    }
}

int main() {
    // 1. Test nested variable access
    int p = physics::constants::pi_scaled;
    print p;

    // 2. Test namespaced function calling nested variable
    int p2 = physics::get_pi();
    print p2;

    // 3. Test global char in namespace
    char u = physics::unit;
    print u;

    return 0;
}
