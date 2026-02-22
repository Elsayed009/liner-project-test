#include <iostream>
#include <string>
using namespace std;

// ========== String Utilities ==========

float myAtof(const string &s) {
    float result = 0;
    float dec = 0;
    bool negative = false;
    bool afterDot = false;
    float decPlace = 0.1f;
    int i = 0;
    if (!s.empty() && s[0] == '-') { negative = true; i = 1; }
    else if (!s.empty() && s[0] == '+') { i = 1; }
    for (; i < (int)s.size(); i++) {
        if (s[i] == '.') { afterDot = true; continue; }
        if (!afterDot) result = result * 10 + (s[i] - '0');
        else { dec += (s[i] - '0') * decPlace; decPlace *= 0.1f; }
    }
    result += dec;
    return negative ? -result : result;
}

string floatToStr(float v) {
    if (v < 0) return "-" + floatToStr(-v);
    int intPart = (int)v;
    float frac = v - intPart;
    string res = "";
    int tmp = intPart;
    string rev = "";
    if (tmp == 0) rev = "0";
    while (tmp > 0) { rev += (char)('0' + tmp % 10); tmp /= 10; }
    for (int i = rev.size()-1; i >= 0; i--) res += rev[i];
    if (frac > 0.0001f) {
        res += ".";
        for (int i = 0; i < 4; i++) {
            frac *= 10;
            res += (char)('0' + (int)frac);
            frac -= (int)frac;
            if (frac < 0.0001f) break;
        }
    }
    return res;
}

// ========== Data Structures ==========

const int MAXVARS = 100;
const int MAXEQS  = 100;

struct Equation {
    string varNames[MAXVARS];
    float  coeffs[MAXVARS];
    int    numVars;
    float  constant;

    Equation() : numVars(0), constant(0) {
        for (int i = 0; i < MAXVARS; i++) coeffs[i] = 0;
    }

    int findVar(const string &v) const {
        for (int i = 0; i < numVars; i++)
            if (varNames[i] == v) return i;
        return -1;
    }

    void addCoeff(const string &v, float c) {
        int idx = findVar(v);
        if (idx == -1) { varNames[numVars] = v; coeffs[numVars] = c; numVars++; }
        else coeffs[idx] += c;
    }

    float getCoeff(const string &v) const {
        int idx = findVar(v);
        return (idx == -1) ? 0 : coeffs[idx];
    }

    void setCoeff(const string &v, float c) {
        int idx = findVar(v);
        if (idx == -1) { varNames[numVars] = v; coeffs[numVars] = c; numVars++; }
        else coeffs[idx] = c;
    }

    void sortVars() {
        for (int i = 0; i < numVars-1; i++)
            for (int j = 0; j < numVars-i-1; j++)
                if (varNames[j] > varNames[j+1]) {
                    string tmpN = varNames[j]; varNames[j] = varNames[j+1]; varNames[j+1] = tmpN;
                    float  tmpC = coeffs[j];   coeffs[j]   = coeffs[j+1];   coeffs[j+1]   = tmpC;
                }
    }
};

// ========== Print Equation ==========

string printEquation(Equation &eq) {
    eq.sortVars();
    string out = "";
    bool first = true;
    for (int i = 0; i < eq.numVars; i++) {
        float c = eq.coeffs[i];
        if (c == 0) continue;
        if (!first && c > 0) out += "+";
        if (c < 0) out += "-" + floatToStr(-c);
        else out += floatToStr(c);
        out += eq.varNames[i];
        first = false;
    }
    if (first) out += "0";
    out += "=";
    if (eq.constant < 0) out += "-" + floatToStr(-eq.constant);
    else out += floatToStr(eq.constant);
    return out;
}

// ========== Parse ==========

void parseLeft(const string &left, Equation &eq) {
    string s = left;
    if (s.empty()) return;
    if (s[0] != '-') s = "+" + s;

    string terms[MAXVARS*2];
    int numTerms = 0;
    string cur = "";
    for (int i = 0; i < (int)s.size(); i++) {
        if ((s[i] == '+' || s[i] == '-') && i != 0) {
            if (!cur.empty()) terms[numTerms++] = cur;
            cur = s[i];
        } else cur += s[i];
    }
    if (!cur.empty()) terms[numTerms++] = cur;

    for (int t = 0; t < numTerms; t++) {
        string &term = terms[t];
        int xPos = -1;
        for (int i = 0; i < (int)term.size(); i++) {
            if (term[i] >= 'a' && term[i] <= 'z') { xPos = i; break; }
        }
        if (xPos == -1) {
            eq.constant -= myAtof(term);
        } else {
            string coeffStr = term.substr(0, xPos);
            string var = term.substr(xPos);
            float coeff;
            if (coeffStr.empty() || coeffStr == "+") coeff = 1;
            else if (coeffStr == "-") coeff = -1;
            else coeff = myAtof(coeffStr);
            eq.addCoeff(var, coeff);
        }
    }
}

// ========== Global ==========

Equation *equations;
int n;

int getAllVars(string *vars) {
    string tmp[MAXVARS];
    int cnt = 0;
    for (int i = 0; i < n; i++)
        for (int j = 0; j < equations[i].numVars; j++) {
            bool found = false;
            for (int k = 0; k < cnt; k++)
                if (tmp[k] == equations[i].varNames[j]) { found = true; break; }
            if (!found) tmp[cnt++] = equations[i].varNames[j];
        }
    for (int i = 0; i < cnt-1; i++)
        for (int j = 0; j < cnt-i-1; j++)
            if (tmp[j] > tmp[j+1]) { string t = tmp[j]; tmp[j] = tmp[j+1]; tmp[j+1] = t; }
    for (int i = 0; i < cnt; i++) vars[i] = tmp[i];
    return cnt;
}

// ========== Determinant ==========

float determinant(float **mat, int sz) {
    if (sz == 1) return mat[0][0];
    if (sz == 2) return mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];
    float det = 0;
    float **sub = new float*[sz-1];
    for (int i = 0; i < sz-1; i++) sub[i] = new float[sz-1];
    for (int p = 0; p < sz; p++) {
        for (int i = 1; i < sz; i++) {
            int col = 0;
            for (int j = 0; j < sz; j++) {
                if (j == p) continue;
                sub[i-1][col++] = mat[i][j];
            }
        }
        det += mat[0][p] * determinant(sub, sz-1) * (p%2==0 ? 1 : -1);
    }
    for (int i = 0; i < sz-1; i++) delete[] sub[i];
    delete[] sub;
    return det;
}

// ========== Main ==========

int main() {
    cin >> n;
    equations = new Equation[n];

    for (int i = 0; i < n; i++) {
        string line;
        cin >> ws;
        getline(cin, line);
        size_t pos = line.find('=');
        string left  = line.substr(0, pos);
        string right = line.substr(pos+1);
        equations[i].constant = myAtof(right);
        parseLeft(left, equations[i]);
        equations[i].sortVars();
    }

    string command;
    while (cin >> command) {
        if (command == "quit") break;

        else if (command == "num_vars") {
            string vars[MAXVARS];
            cout << getAllVars(vars) << endl;
        }

        else if (command == "equation") {
            int idx; cin >> idx;
            cout << printEquation(equations[idx-1]) << endl;
        }

        else if (command == "column") {
            string var; cin >> var;
            for (int i = 0; i < n; i++) {
                float v = equations[i].getCoeff(var);
                if (v == (int)v) cout << (int)v << endl;
                else cout << v << endl;
            }
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
            string var; int eqIdx, subIdx;
            cin >> var >> eqIdx >> subIdx;
            Equation &eq1 = equations[eqIdx-1];
            Equation &eq3 = equations[subIdx-1];
            float c1 = eq1.getCoeff(var);
            float c3 = eq3.getCoeff(var);
            if (c1 == 0) { cout << printEquation(eq1) << endl; continue; }
            if (c3 == 0) { cout << printEquation(eq1) << endl; continue; }
            float factor = c1 / c3;
            Equation res = eq1;
            res.constant -= factor * eq3.constant;
            for (int k = 0; k < eq3.numVars; k++)
                res.addCoeff(eq3.varNames[k], -factor * eq3.coeffs[k]);
            int idx = res.findVar(var);
            if (idx != -1) res.coeffs[idx] = 0;
            cout << printEquation(res) << endl;
        }

        else if (command == "D") {
            string rest; getline(cin, rest);
            string var = "";
            int s = 0;
            while (s < (int)rest.size() && rest[s] == ' ') s++;
            if (s < (int)rest.size()) var = rest.substr(s);

            string vars[MAXVARS];
            int sz = getAllVars(vars);

            float **mat = new float*[n];
            for (int i = 0; i < n; i++) {
                mat[i] = new float[sz];
                for (int j = 0; j < sz; j++) {
                    if (!var.empty() && vars[j] == var)
                        mat[i][j] = equations[i].constant;
                    else
                        mat[i][j] = equations[i].getCoeff(vars[j]);
                }
            }
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < sz; j++) {
                    float v = mat[i][j];
                    if (v == (int)v) cout << (int)v;
                    else cout << v;
                    if (j < sz-1) cout << "\t";
                }
                cout << endl;
            }
            for (int i = 0; i < n; i++) delete[] mat[i];
            delete[] mat;
        }

        else if (command == "D_value") {
            string vars[MAXVARS];
            int sz = getAllVars(vars);
            float **mat = new float*[n];
            for (int i = 0; i < n; i++) {
                mat[i] = new float[sz];
                for (int j = 0; j < sz; j++)
                    mat[i][j] = equations[i].getCoeff(vars[j]);
            }
            float det = determinant(mat, sz);
            if (det == (int)det) cout << (int)det << endl;
            else cout << det << endl;
            for (int i = 0; i < n; i++) delete[] mat[i];
            delete[] mat;
        }

        else if (command == "solve") {
            string vars[MAXVARS];
            int sz = getAllVars(vars);
            float **mat = new float*[n];
            for (int i = 0; i < n; i++) {
                mat[i] = new float[sz];
                for (int j = 0; j < sz; j++)
                    mat[i][j] = equations[i].getCoeff(vars[j]);
            }
            float det = determinant(mat, sz);
            if (det == 0) {
                cout << "No Solution" << endl;
            } else {
                for (int k = 0; k < sz; k++) {
                    float **matk = new float*[n];
                    for (int i = 0; i < n; i++) {
                        matk[i] = new float[sz];
                        for (int j = 0; j < sz; j++)
                            matk[i][j] = (j == k) ? equations[i].constant : mat[i][j];
                    }
                    float detk = determinant(matk, sz);
                    float val = detk / det;
                    cout << vars[k] << "=";
                    if (val == (int)val) cout << (int)val;
                    else cout << val;
                    cout << endl;
                    for (int i = 0; i < n; i++) delete[] matk[i];
                    delete[] matk;
                }
            }
            for (int i = 0; i < n; i++) delete[] mat[i];
            delete[] mat;
        }
    }

    delete[] equations;
    return 0;
}