#include <iostream>
#include <thread>
#include <random>
#include <algorithm>
#include <vector>
#include <mutex>
#include <cmath>
#include <chrono>

// les threads local permettent d'avoir une variable unique à chaque thread
thread_local double valeur_finale_thread = 0.0;
thread_local std::vector<double> valeur_finale_vector;
thread_local std::vector<double> payoff_vector;
thread_local std::vector<double> payoff_put_vector;

class Monte_Carlo {
    private :

    double valeur_initial = 0.0;
    double valeur_finale = 0.0; 
    std::vector<double> valeur_finale_moyenne_vector;
    std::vector<double> payoff_moyenne_vector;
    std::vector<double> payoff_put_moyenne_vector;
    double taux_r = 0.0;
    double sigma = 0;
    int maturite_t = 0;
    int strike = 0;
    double payoff_call = 0.0;
    double payoff_put = 0.0;
    double prix_actualise_call = 0.0;
    double prix_actualise_put = 0.0;
    int nombre_simulations = 0;
    double z = 0.0;
    std::mutex mtx;
    
    public :

    // fonction pour générer une valeur suivant la loi normale (moyenne à 0 et écart-type de 1)
    double générateur_loi_normale() {
        static thread_local std::mt19937 gen(std::random_device{}());
        std::normal_distribution<double> dist(0.0, 1.0);
        return dist(gen);
    }

    // Template (pour int et double) de la fonction payoff 
    template<typename T, typename V>
    auto Payoff(T x, V y) {
        auto payoff = std::max(x, y);
        return payoff;
    }

    void work_thread_simulation() {
        std::lock_guard<std::mutex> lock(mtx); // Utilisation de mutex pour safe les threads
        double somme = 0.0;
        for (int i = 0; i < nombre_simulations; i++) {
            z = générateur_loi_normale();

            // Formule principale pour option européenne
            valeur_finale_thread = valeur_initial * std::exp((taux_r - 0.5 * pow(sigma, 2)) * maturite_t + sigma * sqrt(maturite_t ) * z);

            valeur_finale_vector.push_back(valeur_finale_thread);

            // Calcul Payoff Call
            auto payoff_local = Payoff(valeur_finale_thread - strike, 0.0);
            payoff_vector.push_back(payoff_local);

            // Calcul Payoff Put
            auto payoff_put_local = Payoff(strike - valeur_finale_thread, 0.0);
            payoff_put_vector.push_back(payoff_put_local);
        }

        for (int i = 0; i < valeur_finale_vector.size(); i++) {
            somme += valeur_finale_vector[i];
        }
        
        // Moyenne des simulations (call)
        double moyenne = somme / static_cast<double>(nombre_simulations); 
        valeur_finale_moyenne_vector.push_back(moyenne);

        somme = 0.0;
        moyenne = 0.0;

        for(int i = 0; i < payoff_vector.size(); i++) {
            somme += payoff_vector[i];
        }

        // Moyenne des payoffs (call)
        moyenne = somme / static_cast<double>(nombre_simulations); 
        payoff_moyenne_vector.push_back(moyenne);

        somme = 0;
        moyenne = 0;

        for (int i = 0; i < payoff_put_vector.size(); i++) {
            somme += payoff_put_vector[i];
        }

        // Moyenne des payoffs (put)
        moyenne = somme / static_cast<double>(nombre_simulations);
        payoff_put_moyenne_vector.push_back(moyenne);

    }

    void resultat_finale() {
        double somme = 0.0;
        double nombre_moyenne_valeur_finale = valeur_finale_moyenne_vector.size();
        double nombre_moyenne_payoff = payoff_moyenne_vector.size();
        double nombre_moyenne_payoff_put = payoff_put_moyenne_vector.size();

        for (int i = 0; i < valeur_finale_moyenne_vector.size(); i++) {
            somme += valeur_finale_moyenne_vector[i];
        }
        
        // Moyenne des résultats de chaque thread (call)
        valeur_finale = somme / nombre_moyenne_valeur_finale; 

        somme = 0;

        for (int i = 0; i < nombre_moyenne_payoff; i++) {
            somme += payoff_moyenne_vector[i];
        }

        // Moyenne des payoffs de chaque thread (call)
        payoff_call = somme / nombre_moyenne_payoff; 

        somme = 0;

        for (int i = 0; i < nombre_moyenne_payoff_put; i++) {
            somme += payoff_put_moyenne_vector[i];
        }

        // Moyenne des payoffs de chaque thread (put)
        payoff_put = somme / nombre_moyenne_payoff_put;

        // Actualisation du call et put
        prix_actualise_call = exp(-taux_r * maturite_t) * payoff_call;
        prix_actualise_put = exp(-taux_r * maturite_t) * payoff_put;
    }

    void afficher_interface_debut() {
        std::cout << "-------------- Monte Carlo Calculator Start --------------" << std::endl;
        std::cout << "Donner le prix a t = 0 de l'actif : ";
        std::cin >> valeur_initial;
        std::cout << "\nDonner le strike de l'option : ";
        std::cin >> strike;
        std::cout << "\nDonner le taux d'interet sans risque de l'option : ";
        std::cin >> taux_r;
        std::cout << "\nDonner la maturite T de l'option : ";
        std::cin >> maturite_t;
        std::cout << "\nDonner la volatilite du sous-jacent : ";
        std::cin >> sigma;
        std::cout << "\nDonner le nombre de simulations par thread : ";
        std::cin >> nombre_simulations;
    }

    void afficher_interface_fin() {
        std::cout << "-------------- Monte Carlo Calculator Finish --------------" << std::endl;
        std::cout << "Estimation du prix finale de l'actif : " << valeur_finale << std::endl;
        std::cout << "Estimation du payoff (call) finale de l'option : " << payoff_call << std::endl;
        std::cout << "Estimation du payoff (put) finale de l'option : " << payoff_put << std::endl;
        std::cout << "Estimation du prix actualise (call) : " << prix_actualise_call << std::endl;
        std::cout << "Estimation du prix actualise (put) : " << prix_actualise_put << std::endl;
    }


};

int main() {
    Monte_Carlo m_c;
    m_c.afficher_interface_debut();
    auto start = std::chrono::high_resolution_clock::now(); // début du chrono
    std::thread t1(&Monte_Carlo::work_thread_simulation, &m_c);
    std::thread t2(&Monte_Carlo::work_thread_simulation, &m_c);
    std::thread t3(&Monte_Carlo::work_thread_simulation, &m_c);
    std::thread t4(&Monte_Carlo::work_thread_simulation, &m_c);
    std::thread t5(&Monte_Carlo::work_thread_simulation, &m_c);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    auto finish = std::chrono::high_resolution_clock::now(); // fin du chrono
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    m_c.resultat_finale();
    m_c.afficher_interface_fin();
    std::cout << "Temps d'execution : " << duration.count() << " (ms)";

    return 0;
}