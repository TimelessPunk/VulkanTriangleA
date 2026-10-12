#include "Application.h"
#include "Renderer.h"

void Application::Init()
{
    InitWindow();
    InitVulkan();
}

void Application::Run()
{
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
    }
}

void Application::Destroy()
{
    DestroyVulkan();
    DestroyWindow();
}

void Application::InitWindow()
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(800, 600, "Vulkan Triangle", nullptr, nullptr);
}

void Application::InitVulkan()
{
    &Renderer::CreateInstance;
    &Renderer::PickPhysicalDevice;
    &Renderer::CreateLogicalDevice;
}

void Application::DestroyWindow()
{
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Application::DestroyVulkan()
{

}



