#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <memory>
#include <algorithm>
#include "minisat/core/SolverTypes.h"
#include "minisat/core/Solver.h"
#include <pthread.h>
#include <time.h>


using namespace std;

int N=0;
const int MAX = 100;
vector<int> vertices[MAX];
string err_msg;
vector<int> vertex_cover1;
vector<int> vertex_cover2;
vector<int> vertex_cover3;

void print_error(){
    cerr << "Error: " << err_msg << "\n";
    for(int i=0; i<N; i++){
        vertices[i].clear();
    }
}

void* cnf_sat(void* arg){

    int min = 1;
    int max = N;
    int n = N;
    vertex_cover1.clear();
    while(min <= max){
        int k = floor((min+max)/2);

        unique_ptr<Minisat::Solver> solver(new Minisat::Solver());
        Minisat::vec<Minisat::Lit> clause;
        vector< vector<Minisat::Lit> > literals(n,vector<Minisat::Lit>(n));
        literals.clear();
        literals.resize(n,vector<Minisat::Lit>(n));
        

        //Creating nxk Matrix
        for (int i = 0;i<n;i++)
        {
            for (int j=0;j<k;j++)
            {
                literals[i][j] = (Minisat::mkLit(solver->newVar()));	
            }
        }

        //clause 1
        for (int i=0; i<k;i++)
        {
            for(int j=0;j<n;j++)
            {	
                clause.push(literals[j][i]);
            }
            solver->addClause(clause);
            clause.clear();

        }

        //clause 2
        for (int m=0; m<n;m++)
        {
            for (int p=0;p<k-1;p++)
            {	
                for(int q=p+1;q<k;q++)
                {
                    solver->addClause(~literals[m][p],~literals[m][q]);
                    //(¬x m,p ? ¬x m,q )
                }
            }
        }

        //clause 3
        for (int m=0; m<k;m++)
        {
            for(int p=0;p<n-1;p++)
            {
                for(int q=p+1;q<n;q++)
                {
                    solver->addClause(~literals[p][m],~literals[q][m]);             
                    //(¬x p,m ? ¬x q,m )	
                }
            }
        }

        //clause 4
        for(int l=0; l<n;l++){
            if(vertices[l].size()!=0){
                for(unsigned int n=0; n<vertices[l].size(); n++){
                    if(l>vertices[l][n]) continue;
                    for(int m=0; m<k; m++)
                    {
                        clause.push(literals[l][m]);
                        clause.push(literals[(vertices[l][n])][m]);
                    }
                    solver->addClause(clause);
                    clause.clear();
                
                }
            }

        }

        //create clauses to fix the order of vertices in the solution
        for (unsigned i1 = 0; i1 < n; i1++){
            for(unsigned j1 = 0; j1 < k; j1++){
                for(unsigned i2 = i1+1; i2 < n; i2++){
                   for(unsigned j2 = 0; j2 < j1; j2++){
                       solver->addClause(~literals[i1][j1],~literals[i2][j2]);
                   }
                }
            }
        }

        bool res = solver->solve();
        // cout << "The result is: " << res << "\n";
        if(res){
            vertex_cover1.clear();
            for (int i = 0;i<n;i++)
            {
                for (int j=0;j<k;j++)
                {
                    if(Minisat::toInt(solver->modelValue(literals[i][j]))==0)
                    {   
                    vertex_cover1.push_back(i);
                    }
                }       
            }
        }
        solver.reset (new Minisat::Solver());
        if(!res){
            min = k + 1;
        }
        else{
            max = k - 1;
        }
    }
    return nullptr;
}

vector<vector<int>> create_matrix(){

    vector<vector<int> > matrix(N);
    for ( int i = 0 ; i < N ; i++ )
        matrix[i].resize(N);

    for (int j=0; j<N; j++){
        if(vertices[j].size()!=0){
            for(unsigned int k=0; k<vertices[j].size(); k++){
                matrix[j][vertices[j][k]] = 1;
            }
        }
    }
    return matrix;
}

void* approx_vc1(void* arg){

    vector< vector<int>> adj_matrix = create_matrix();
    vertex_cover2.clear();

    while(true) {
        // choosing highest degree vertex
        int vertex = -1;
        int max_degree = 0;
        for (unsigned int l=0; l < adj_matrix.size(); l++) {
            int temp = 0;
            for (auto& k : adj_matrix[l])
                temp += k;
             
            if (temp > max_degree) {
                vertex = l;
                max_degree = temp;
            }
        }
        
        if (vertex < 0) {
            break;
        }

        //Remove edges incident to vertex
        for (unsigned int i=0; i < adj_matrix.size(); i++) {
            adj_matrix[i][vertex] = 0; // set column to 0
            adj_matrix[vertex][i] = 0; // set row to 0
        }

        vertex_cover2.push_back(vertex);
        
    }
    sort(vertex_cover2.begin(), vertex_cover2.end());
    return nullptr;
}

void* approx_vc2(void* arg){

    vector< vector<int>> adj_matrix = create_matrix();
    vertex_cover3.clear();

    while(true) {
        int i;
        int j;
        bool found = false;
        for (int l=0; l < N; l++) {
            for (int k=0; k < N; k++) {
                if (adj_matrix[l][k] == 1) {
                    i = l;
                    j = k;
                    found = true;
                    break;
                } 
            }
            if (found){
                break;
            }
        }
        if (!found) {
            break;
        }

        vertex_cover3.push_back(i);
        vertex_cover3.push_back(j);

        //Remove edges incident to vertices
        for (unsigned int edge=0; edge < adj_matrix.size(); edge++) {
            adj_matrix[i][edge] = 0; //Set row to 0
            adj_matrix[j][edge] = 0; //Set row to 0       
            adj_matrix[edge][i] = 0; //Set column to 0
            adj_matrix[edge][j] = 0; //Set column to 0     

        }
    }
    sort(vertex_cover3.begin(), vertex_cover3.end());
    return nullptr;
}

void print_vertexcover(vector<int> vertex_cover){
    for (unsigned int i =0; i<vertex_cover.size();i++)
    {
        if (i<vertex_cover.size()-1)
        {
            cout<<vertex_cover[i]<<",";
        }
        else
        {
            cout<<vertex_cover[i];
        }
    }
    cout<<endl;
}

void* parse_line (void* arg)
{
    while (!cin.eof()) {
        string line;
        getline(cin, line);
        if(cin.eof()){
            return nullptr;
        }
        istringstream input(line);
        char ch;
        input >> ch;

        switch(ch){
            case 'V':
                if(N){
                    for(int i=0;i<N;i++){
                        vertices[i].clear();
                    }
                }
                input >> N;
                break;
            case 'E':
                ws(input);
                int node1, node2;
                char a1,a2,a3,a4;
                input >> a1;
                while(input >> a1){
                    input >> node1 >> a2 >> node2 >> a3 >> a4;
                    if(node1<N && node2<N){
                        vertices[node1].push_back(node2);
                        vertices[node2].push_back(node1);
                    }
                    else{
                        err_msg = "Invalid Input";
                        print_error();
                        break;
                    }
                }
                pthread_t cnf;
                pthread_create(&cnf, nullptr,cnf_sat,nullptr);
                pthread_t approx1;
                pthread_create(&approx1, nullptr,approx_vc1,nullptr);
                pthread_t approx2;
                pthread_create(&approx2, nullptr,approx_vc2,nullptr);
                pthread_join(cnf,nullptr);
                pthread_join(approx1,nullptr);
                pthread_join(approx2,nullptr);
                cout << "CNF-SAT-VC: ";
                print_vertexcover(vertex_cover1);
                cout << "APPROX-VC-1: ";
                print_vertexcover(vertex_cover2);
                cout << "APPROX-VC-2: ";
                print_vertexcover(vertex_cover3);
                break;
            default:
                err_msg = "Unknown Command";
                print_error();
                break;
        }
    }
    return nullptr;
}

int main() {
    pthread_t io;
    int iortn;
    //use vertices to store graph in adjacency-list representation
    // read from stdin until EOF
    iortn = pthread_create(&io,nullptr,parse_line,nullptr);
    if(iortn !=0){
        cerr <<"Error: Failed to create pthread for io function." <<endl;
    }
    iortn = pthread_join(io, nullptr);
    if(iortn !=0){
        cerr << "Error: Failed to join io function thread." <<endl;
    }
    return 0;
}