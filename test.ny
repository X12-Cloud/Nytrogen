#include <complex.nyt>

int main() {
    complex z = 3.0 + 4.0i;
    print mcomplex::conj(z);     // (3.0000 + -4.0000i)
    print mcomplex::norm_sq(z);  // 25.0000

    matrix X = mcomplex::pauli_x();
    matrix Z = mcomplex::pauli_z();

    matrix XZ = mcomplex::dot(X, Z);
    print XZ;

    matrix H = mcomplex::hadamard();
    print mcomplex::det(H);

    return 0;
}
