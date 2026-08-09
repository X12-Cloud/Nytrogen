namespace App {
    int version = 1;

    namespace Math {
        int version = 2;

        int add(int a, int b) {
            return a + b;
        }
    }

    namespace Physics {
        int gravity = 9;

        int get_total_version() {
            return App::version + App::Math::version;
        }
    }
}

int main() {
    print "App Version: ", App::version;
    print "Math Version: ", App::Math::version;
    print "Gravity: ", App::Physics::gravity;

    print "Total: ";
    //print App::Physics::get_total_version();

    return 0;
}
