#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib>
using namespace std;

struct Equation {
    string* varNames;
    float*  coeffs;
    int     numVars;
    int     capacity;
    float   constant;

    Equation() : numVars(0), capacity(10), constant(0) {
        varNames = new string[capacity];
        coeffs   = new float[capacity];
        for (int i = 0; i < capacity; i++) coeffs[i] = 0;
    }

    Equation(const Equation& other) : numVars(other.numVars), capacity(other.capacity), constant(other.constant) {
        varNames = new string[capacity];
        coeffs   = new float[capacity];
        for (int i = 0; i < capacity; i++) {
            varNames[i] = other.varNames[i];
            coeffs[i]   = other.coeffs[i];
        }
    }

    Equation& operator=(const Equation& other) {
        if (this == &other) return *this;
        delete[] varNames;
        delete[] coeffs;
        numVars  = other.numVars;
        capacity = other.capacity;
        constant = other.constant;
        varNames = new string[capacity];
        coeffs   = new float[capacity];
        for (int i = 0; i < capacity; i++) {
            varNames[i] = other.varNames[i];
            coeffs[i]   = other.coeffs[i];
        }
        return *this;
    }

    ~Equation() {
        delete[] varNames;
        delete[] coeffs;
    }

    void grow() {
        capacity *= 2;
        string* newNames = new string[capacity];
        float*  newCoefs = new float[capacity];
        for (int i = 0; i < numVars; i++) {
            newNames[i] = varNames[i];
            newCoefs[i] = coeffs[i];
        }
        delete[] varNames;
        delete[] coeffs;
        varNames = newNames;
        coeffs   = newCoefs;
    }

    int findVar(const string& v) const {
        for (int i = 0; i < numVars; i++)
            if (varNames[i] == v) return i;
        return -1;
    }

    void addCoeff(const string& v, float c) {
        int idx = findVar(v);
        if (idx == -1) {
            if (numVars == capacity) grow();
            varNames[numVars] = v;
            coeffs[numVars]   = c;
            numVars++;
        } else {
            coeffs[idx] += c;
        }
    }

    float getCoeff(const string& v) const {
        int idx = findVar(v);
        return (idx == -1) ? 0 : coeffs[idx];
    }

    void sortVars() {
        for (int i = 0; i < numVars - 1; i++)
            for (int j = 0; j < numVars - i - 1; j++)
                if (varNames[j] > varNames[j+1]) {
                    string tn = varNames[j]; varNames[j] = varNames[j+1]; varNames[j+1] = tn;
                    float  tc = coeffs[j];   coeffs[j]   = coeffs[j+1];   coeffs[j+1]   = tc;
                }
    }
};

string printEquation(Equation& eq) {
    eq.sortVars();
    ostringstream out;
    bool first = true;
    for (int i = 0; i < eq.numVars; i++) {
        float c = eq.coeffs[i];
        if (c == 0) continue;
        if (!first && c > 0) out << "+";
        if (c == (int)c) out << (int)c;
        else              out << c;
        out << eq.varNames[i];
        first = false;
    }
    if (first) out << "0";
    out << "=";
    if (eq.constant == (int)eq.constant) out << (int)eq.constant;
    else                                  out << eq.constant;
    return out.str();
}

void parseLeft(const string& left, Equation& eq) {
    string s = left;
    if (s.empty()) return;
    if (s[0] != '-') s = "+" + s;

    string* terms    = new string[200];
    int     numTerms = 0;
    string  cur      = "";

    for (int i = 0; i < (int)s.size(); i++) {
        if ((s[i] == '+' || s[i] == '-') && i != 0) {
            if (!cur.empty()) terms[numTerms++] = cur;
            cur = s[i];
        } else cur += s[i];
    }
    if (!cur.empty()) terms[numTerms++] = cur;

    for (int t = 0; t < numTerms; t++) {
        string& term = terms[t];
        int pos = -1;
        for (int i = 0; i < (int)term.size(); i++)
            if (term[i] >= 'a' && term[i] <= 'z') { pos = i; break; }
        if (pos == -1) {
            eq.constant -= atof(term.c_str());
        } else {
            string coefStr = term.substr(0, pos);
            string var     = term.substr(pos);
            float coef;
            if (coefStr.empty() || coefStr == "+") coef = 1;
            else if (coefStr == "-")               coef = -1;
            else                                   coef = atof(coefStr.c_str());
            eq.addCoeff(var, coef);
        }
    }
    delete[] terms;
}

Equation* equations;
int n;

int getAllVars(string*& vars) {
    int    cap = 10;
    string* tmp = new string[cap];
    int     cnt = 0;

    for (int i = 0; i < n; i++)
        for (int j = 0; j < equations[i].numVars; j++) {
            bool found = false;
            for (int k = 0; k < cnt; k++)
                if (tmp[k] == equations[i].varNames[j]) { found = true; break; }
            if (!found) {
                if (cnt == cap) {
                    cap *= 2;
                    string* tmp2 = new string[cap];
                    for (int x = 0; x < cnt; x++) tmp2[x] = tmp[x];
                    delete[] tmp;
                    tmp = tmp2;
                }
                tmp[cnt++] = equations[i].varNames[j];
            }
        }

    for (int i = 0; i < cnt - 1; i++)
        for (int j = 0; j < cnt - i - 1; j++)
            if (tmp[j] > tmp[j+1]) { string t = tmp[j]; tmp[j] = tmp[j+1]; tmp[j+1] = t; }

    vars = tmp;
    return cnt;
}

float determinant(float** mat, int sz) {
    if (sz == 1) return mat[0][0];
    if (sz == 2) return mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];
    float result = 0;
    float** sub  = new float*[sz-1];
    for (int i = 0; i < sz-1; i++) sub[i] = new float[sz-1];
    for (int p = 0; p < sz; p++) {
        for (int i = 1; i < sz; i++) {
            int col = 0;
            for (int j = 0; j < sz; j++)
                if (j != p) sub[i-1][col++] = mat[i][j];
        }
        result += mat[0][p] * determinant(sub, sz-1) * (p%2==0 ? 1 : -1);
    }
    for (int i = 0; i < sz-1; i++) delete[] sub[i];
    delete[] sub;
    return result;
}

float** buildMatrix(string* vars, int sz, string replaceVar = "") {
    float** mat = new float*[n];
    for (int i = 0; i < n; i++) {
        mat[i] = new float[sz];
        for (int j = 0; j < sz; j++)
            mat[i][j] = (!replaceVar.empty() && vars[j] == replaceVar)
                        ? equations[i].constant : equations[i].getCoeff(vars[j]);
    }
    return mat;
}

void freeMatrix(float** mat) {
    for (int i = 0; i < n; i++) delete[] mat[i];
    delete[] mat;
}

void printVal(float v) {
    if (v == (int)v) cout << (int)v;
    else             cout << v;
}

int main() {
    cin >> n;
    equations = new Equation[n];

    for (int i = 0; i < n; i++) {
        string line;
        cin >> ws;
        getline(cin, line);
        size_t pos        = line.find('=');
        equations[i].constant = atof(line.substr(pos+1).c_str());
        parseLeft(line.substr(0, pos), equations[i]);
        equations[i].sortVars();
    }

    string command;
    while (cin >> command) {
        if (command == "quit") break;

        else if (command == "num_vars") {
            string* vars = nullptr;
            int cnt = getAllVars(vars);
            cout << cnt << endl;
            delete[] vars;
        }

        else if (command == "equation") {
            int idx; cin >> idx;
            cout << printEquation(equations[idx-1]) << endl;
        }

        else if (command == "column") {
            string var; cin >> var;
            for (int i = 0; i < n; i++) { printVal(equations[i].getCoeff(var)); cout << endl; }
        }

        else if (command == "add") {
            int i, j; cin >> i >> j;
            Equation res = equations[i-1];
            res.constant += equations[j-1].constant;
            for (int k = 0; k < equations[j-1].numVars; k++)
                res.addCoeff(equations[j-1].varNames[k], equations[j-1].coeffs[k]);
            cout << printEquation(res) << endl;
        }

        else if (command == "subtract") {
            int i, j; cin >> i >> j;
            Equation res = equations[i-1];
            res.constant -= equations[j-1].constant;
            for (int k = 0; k < equations[j-1].numVars; k++)
                res.addCoeff(equations[j-1].varNames[k], -equations[j-1].coeffs[k]);
            cout << printEquation(res) << endl;
        }

        else if (command == "substitute") {
            string var; int a, b; cin >> var >> a >> b;
            Equation& eq1 = equations[a-1];
            Equation& eq2 = equations[b-1];
            float c1 = eq1.getCoeff(var), c2 = eq2.getCoeff(var);
            if (c1 == 0 || c2 == 0) { cout << printEquation(eq1) << endl; continue; }
            float factor = c1 / c2;
            Equation res = eq1;
            res.constant -= factor * eq2.constant;
            for (int k = 0; k < eq2.numVars; k++)
                res.addCoeff(eq2.varNames[k], -factor * eq2.coeffs[k]);
            int idx = res.findVar(var);
            if (idx != -1) res.coeffs[idx] = 0;
            cout << printEquation(res) << endl;
        }

        else if (command == "D") {
            string rest, var = "";
            getline(cin, rest);
            stringstream ss(rest);
            ss >> var;

            string* vars = nullptr;
            int sz = getAllVars(vars);
            float** mat = buildMatrix(vars, sz, var);
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < sz; j++) {
                    printVal(mat[i][j]);
                    if (j < sz-1) cout << "\t";
                }
                cout << endl;
            }
            freeMatrix(mat);
            delete[] vars;
        }

        else if (command == "D_value") {
            string* vars = nullptr;
            int sz = getAllVars(vars);
            float** mat = buildMatrix(vars, sz);
            printVal(determinant(mat, sz));
            cout << endl;
            freeMatrix(mat);
            delete[] vars;
        }

        else if (command == "solve") {
            string* vars = nullptr;
            int sz = getAllVars(vars);
            float** mat = buildMatrix(vars, sz);
            float d = determinant(mat, sz);
            if (d == 0) {
                cout << "No Solution" << endl;
            } else {
                for (int k = 0; k < sz; k++) {
                    float** matk = buildMatrix(vars, sz, vars[k]);
                    cout << vars[k] << "=";
                    printVal(determinant(matk, sz) / d);
                    cout << endl;
                    freeMatrix(matk);
                }
            }
            freeMatrix(mat);
            delete[] vars;
        }
    }

    delete[] equations;
    return 0;
}