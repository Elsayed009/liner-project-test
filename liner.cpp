#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <cstdlib>
#include <cmath>
using namespace std;

struct Equation {
    map<string, float> coeffs;
    float constant;
};

// طباعة المعادلة - المعامل 1 و-1 بيتطبعوا كـ 1x و-1x
string printEquation(const Equation &eq) {
    ostringstream out;
    bool first = true;
    for (auto &p : eq.coeffs) {
        float c = p.second;
        if (c == 0) continue;
        if (!first && c > 0) out << "+";
        // نطبع المعامل دايما حتى لو 1 أو -1
        // لكن بنتخلص من الـ .0 في حالة أرقام صحيحة
        if (c == (int)c) {
            out << (int)c;
        } else {
            out << c;
        }
        out << p.first;
        first = false;
    }
    if (first) out << "0"; // لو مفيش حدود
    out << "=";
    if (eq.constant == (int)eq.constant) out << (int)eq.constant;
    else out << eq.constant;
    return out.str();
}

// حساب المحدد
float determinant(vector<vector<float>> mat) {
    int n = mat.size();
    if (n == 1) return mat[0][0];
    if (n == 2) return mat[0][0]*mat[1][1] - mat[0][1]*mat[1][0];
    float det = 0;
    for (int p = 0; p < n; p++) {
        vector<vector<float>> submat(n-1, vector<float>(n-1));
        for (int i = 1; i < n; i++) {
            int col = 0;
            for (int j = 0; j < n; j++) {
                if (j == p) continue;
                submat[i-1][col] = mat[i][j];
                col++;
            }
        }
        det += mat[0][p] * determinant(submat) * (p%2==0 ? 1 : -1);
    }
    return det;
}

// parse term واحد (مثل 2x1 أو -3x2 أو x3 أو -x1)
// بيرجع variable name والـ coefficient
// لو مش متغير بيرجع "" والقيمة العددية
pair<string, float> parseTerm(const string &term) {
    size_t xPos = string::npos;
    // دور على أول حرف حروف
    for (size_t i = 0; i < term.size(); i++) {
        if (isalpha(term[i])) {
            xPos = i;
            break;
        }
    }
    if (xPos == string::npos) {
        // constant term
        return {"", atof(term.c_str())};
    }
    string coeffStr = term.substr(0, xPos);
    string var = term.substr(xPos);
    float coeff;
    if (coeffStr.empty() || coeffStr == "+") coeff = 1;
    else if (coeffStr == "-") coeff = -1;
    else coeff = atof(coeffStr.c_str());
    return {var, coeff};
}

// parse الجانب الأيسر من المعادلة مع دعم الطرح
void parseLeft(const string &left, map<string,float> &coeffs, float &constant) {
    // نقسم على + و-، مع الحفاظ على الإشارة
    string s = left;
    // نضيف + في البداية لو مبدأش بـ -
    if (s.empty()) return;
    if (s[0] != '-') s = "+" + s;
    
    vector<string> terms;
    string cur = "";
    for (size_t i = 0; i < s.size(); i++) {
        if ((s[i] == '+' || s[i] == '-') && i != 0) {
            if (!cur.empty()) terms.push_back(cur);
            cur = s[i];
        } else {
            cur += s[i];
        }
    }
    if (!cur.empty()) terms.push_back(cur);
    
    for (auto &t : terms) {
        auto p = parseTerm(t);
        if (p.first.empty()) {
            constant -= p.second; // ننقل الثابت للجانب الأيمن
        } else {
            coeffs[p.first] += p.second;
        }
    }
}

int main() {
    int n;
    cin >> n;
    vector<Equation> equations(n);

    // قراءة المعادلات
    for (int i = 0; i < n; i++) {
        string line;
        cin >> ws;
        getline(cin, line);

        Equation eq;
        size_t pos = line.find('=');
        string left = line.substr(0, pos);
        string right = line.substr(pos+1);
        eq.constant = atof(right.c_str());

        parseLeft(left, eq.coeffs, eq.constant);
        equations[i] = eq;
    }

    // جمع كل المتغيرات الموجودة في كل المعادلات
    auto getAllVars = [&]() {
        map<string,bool> varMap;
        for (auto &eq : equations)
            for (auto &p : eq.coeffs) varMap[p.first] = true;
        vector<string> vars;
        for (auto &p : varMap) vars.push_back(p.first);
        return vars;
    };

    // تنفيذ الأوامر
    string command;
    while (cin >> command) {
        if (command == "quit") break;

        else if (command == "num_vars") {
            cout << getAllVars().size() << endl;
        }

        else if (command == "equation") {
            int idx; cin >> idx;
            cout << printEquation(equations[idx-1]) << endl;
        }

        else if (command == "column") {
            string var; cin >> var;
            for (auto &eq : equations) {
                auto it = eq.coeffs.find(var);
                float val = (it != eq.coeffs.end()) ? it->second : 0;
                if (val == (int)val) cout << (int)val << endl;
                else cout << val << endl;
            }
        }

        else if (command == "add") {
            int i,j; cin >> i >> j;
            Equation res;
            res.constant = equations[i-1].constant + equations[j-1].constant;
            res.coeffs = equations[i-1].coeffs;
            for (auto &p : equations[j-1].coeffs)
                res.coeffs[p.first] += p.second;
            cout << printEquation(res) << endl;
        }

        else if (command == "subtract") {
            int i,j; cin >> i >> j;
            Equation res;
            res.constant = equations[i-1].constant - equations[j-1].constant;
            res.coeffs = equations[i-1].coeffs;
            for (auto &p : equations[j-1].coeffs) {
                res.coeffs[p.first] -= p.second;
            }
            // نضيف أي متغير في j مش موجود في i بقيمة 0
            for (auto &p : equations[j-1].coeffs) {
                if (res.coeffs.find(p.first) == res.coeffs.end())
                    res.coeffs[p.first] = -p.second;
            }
            cout << printEquation(res) << endl;
        }

        else if (command == "substitute") {
            // substitute x2 1 3
            // يستبدل x2 في معادلة 1 بمعادلة 3
            string var; int eqIdx, subIdx;
            cin >> var >> eqIdx >> subIdx;
            
            Equation &eq1 = equations[eqIdx-1];
            Equation &eq3 = equations[subIdx-1];
            
            // لو x2 مش موجود في eq1، نطبعها كما هي
            if (eq1.coeffs.find(var) == eq1.coeffs.end() || eq1.coeffs[var] == 0) {
                cout << printEquation(eq1) << endl;
                continue;
            }
            
            // معامل var في eq1
            float c1 = eq1.coeffs[var];
            // معامل var في eq3
            float c3 = 0;
            if (eq3.coeffs.find(var) != eq3.coeffs.end())
                c3 = eq3.coeffs[var];
            
            if (c3 == 0) {
                // مينفعش نحل
                cout << printEquation(eq1) << endl;
                continue;
            }
            
            // نضرب eq3 في (c1/c3) ونطرحها من eq1
            float factor = c1 / c3;
            Equation res;
            res.coeffs = eq1.coeffs;
            res.constant = eq1.constant - factor * eq3.constant;
            for (auto &p : eq3.coeffs) {
                res.coeffs[p.first] -= factor * p.second;
            }
            // نشيل var اللي اتعملتله substitute (المفروض يبقى 0)
            res.coeffs.erase(var);
            
            cout << printEquation(res) << endl;
        }

        else if (command == "D") {
            string rest;
            getline(cin, rest);
            // نشوف لو في variable اسمه بعد D
            stringstream ss(rest);
            string var; ss >> var;
            
            vector<string> vars;
            for (auto &p : equations[0].coeffs) vars.push_back(p.first);
            
            // لو مفيش متغيرات من equation[0]، نجيب كل المتغيرات
            if (vars.empty()) {
                vars = getAllVars();
            }
            
            int sz = vars.size();
            vector<vector<float>> mat(n, vector<float>(sz, 0));
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < sz; j++) {
                    if (!var.empty() && vars[j] == var)
                        mat[i][j] = equations[i].constant;
                    else {
                        auto it = equations[i].coeffs.find(vars[j]);
                        mat[i][j] = (it != equations[i].coeffs.end()) ? it->second : 0;
                    }
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
        }

        else if (command == "D_value") {
            vector<string> vars;
            for (auto &p : equations[0].coeffs) vars.push_back(p.first);
            int sz = vars.size();
            vector<vector<float>> mat(n, vector<float>(sz, 0));
            for (int i = 0; i < n; i++)
                for (int j = 0; j < sz; j++) {
                    auto it = equations[i].coeffs.find(vars[j]);
                    mat[i][j] = (it != equations[i].coeffs.end()) ? it->second : 0;
                }
            float det = determinant(mat);
            if (det == (int)det) cout << (int)det << endl;
            else cout << det << endl;
        }

        else if (command == "solve") {
            vector<string> vars;
            for (auto &p : equations[0].coeffs) vars.push_back(p.first);
            int sz = vars.size();
            vector<vector<float>> mat(n, vector<float>(sz, 0));
            for (int i = 0; i < n; i++)
                for (int j = 0; j < sz; j++) {
                    auto it = equations[i].coeffs.find(vars[j]);
                    mat[i][j] = (it != equations[i].coeffs.end()) ? it->second : 0;
                }
            float det = determinant(mat);
            if (det == 0) {
                cout << "No Solution" << endl;
            } else {
                for (int k = 0; k < sz; k++) {
                    vector<vector<float>> matk = mat;
                    for (int i = 0; i < n; i++)
                        matk[i][k] = equations[i].constant;
                    float detk = determinant(matk);
                    float val = detk/det;
                    cout << vars[k] << "=";
                    if (val == (int)val) cout << (int)val;
                    else cout << val;
                    cout << endl;
                }
            }
        }
    }
    return 0;
}