/*********************************************************************
 * @file  Apllication.hpp
 * @author Florian Houssin <houssin-de-s@insa-toulouse.fr>
 * @brief Fichier header de l'application
 *********************************************************************/
#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

/**
 * @class Application
 * @brief Classe Application
 */
class Application
{
public:
    /**
     * @fn Application();
     * @brief Constructeur par defaut
     */
    Application();
    /**
     * @fn Application();
     * @brief Destructeur
     */
    ~Application();
    /**
     * @fn void init(void)
     * @brief Fonction d'initialisation de l'application
     */
    void init(void);
    /**
     * @fn void run(void)
     * @brief Fonction de lancement de l'application
     */
    void run(void);

private:

};

#endif
