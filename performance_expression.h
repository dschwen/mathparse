const std::string expression =
    "(c * (1 - c) * (0.380577499303532 + -0.0885190880468175 * (1 - 2 * c) + \
                                -0.0549325534161349 * (1 - 2 * c) ^\
                            2 + -0.203752174307515 * (1 - 2 * c) ^\
                            3 + -0.164028794268281 * (1 - 2 * c) ^\
                            4 + 0.0172550241855144 * (1 - 2 * c) ^ 5) +\
             c * -3.85953051397515 + (1 - c) * -4.16723234898182) *\
            y / T0 +\
        (c * (1 - c) * (0.385512095260579 + -0.0962430189496054 * (1 - 2 * c) +\
                            -0.041249704177587 * (1 - 2 * c) ^\
                        2 + -0.194439246552959 * (1 - 2 * c) ^\
                        3 + -0.195412847295217 * (1 - 2 * c) ^\
                        4 + -0.00967038578662529 * (1 - 2 * c) ^ 5) +\
         c * -3.83660056975999 + (1 - c) * -4.12297413272388) -\
        (0.000263056717498972 + 4.614980081531 * 10 ^ -5 * (1 - c) + -4.75235048526914 * 10 ^\
         -5 * (1 - c) ^ 2 + 9.35759929354588 * 10 ^ -6 * (1 - c) ^ 3) *\
            y * log(y) -\
        (3.04676203180853 * 10 ^ -9 + -2.07225774483557 * 10 ^\
         -8 * (1 - c) + 3.55582178830517 * 10 ^ -8 * (1 - c) ^ 2 + -2.70425743485173 * 10 ^\
         -8 * (1 - c) ^ 3) *\
            y ^\
    2 +\
        (-1.51823088659839 * 10 ^ -13 + 5.18553402098699 * 10 ^\
         -12 * (1 - c) + -4.56309143596694 * 10 ^ -12 * (1 - c) ^ 2 + 1.08597105154957 * 10 ^\
         -11 * (1 - c) ^ 3) /\
            2 * y ^\
    3 + (-(c * (1 - c) * (\
                   0.385512095260579\
                + -0.0962430189496054 * (1 - 2*c)\
                + -0.041249704177587  * (1 - 2*c)^2\
                + -0.194439246552959  * (1 - 2*c)^3\
                + -0.195412847295217  * (1 - 2*c)^4\
                + -0.00967038578662529* (1 - 2*c)^5\
            )\
          + c * -3.83660056975999 + (1-c) * -4.12297413272388) / T0 + (0.000263056717498972\
                +  4.614980081531*10^-5   * (1-c)\
                + -4.75235048526914*10^-5 * (1-c)^2\
                +  9.35759929354588*10^-6 * (1-c)^3) * log(T0) + (3.04676203180853*10^-9\
                      + -2.07225774483557*10^-8 * (1-c)\
                      +  3.55582178830517*10^-8 * (1-c)^2\
                      + -2.70425743485173*10^-8 * (1-c)^3) * T0 + (-1.51823088659839*10^-13\
                            +  5.18553402098699*10^-12 * (1-c)\
                            + -4.56309143596694*10^-12 * (1-c)^2\
                            + 1.08597105154957*10^-11  * (1-c)^3) / 2 * T0 ^ 2) * y +\
        y * kB * ((1 - c) * log(1 - c) + c * log(c))";