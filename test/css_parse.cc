#include <iostream>

#include <libcss/stylesheet.h>

int main(int argc, char const **argv)
{
    css_stylesheet *stylesheet;
    css_stylesheet_params params;

    css_error error = css_stylesheet_create(&params, &stylesheet);
    if (error != CSS_OK)
    {
        std::cerr << css_error_to_string(error) << std::endl;
        return 1;
    }

    css_stylesheet_destroy(stylesheet);
    return 0;
}
