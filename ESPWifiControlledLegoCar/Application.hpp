/*********************************************************************
 * @file  Application.hpp
 * @author Florian Houssin <houssin-de-s@insa-toulouse.fr>
 * @brief Application header file
 *********************************************************************/
#ifndef APPLICATION_HPP_
#define APPLICATION_HPP_

/**
 * @class Application
 * @brief Project wide application
 */
class Application
{
public:
    /**
     * @fn Application()
     * @brief Default constructor
     */
    Application();
    /**
     * @fn Application()
     * @brief Destructor
     */
    ~Application();
    /**
     * @fn void Init()
     * @brief Initialize application
     */
    void Init();
    /**
     * @fn void Update()
     * @brief Run the application update loop once
     */
    void Update();

private:

};

#endif
