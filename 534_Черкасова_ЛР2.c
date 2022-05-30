/*
    Лабораторна робота № 2
    Черкасова Марія
    534 група
*/

#define SIMULATION

#include <avr/io.h>

#include <stdbool.h>

// Частота
#define F_CPU 16000000UL
#include <util/delay.h>

// Умикач пральної машинки
#define ENGINE_BUTTON 0b1

// Кнопка для зміни режиму
#define MODE_BUTTON 0b10

// Один оберт ротора у кроках
#define STEPS_PER_SPIN 1024

// Затримки
#define SMALL_DELAY 30
#define BIG_DELAY 60

// Кількість обмоток
#define COILS_NUMBER 4

// Кількість поворотів для делікатного режиму
#define DELICATE_TURNS 8

// Кількість поворотів для нормального режиму
#define NORMAL_TURNS 15

// Режими прання
enum WashingMode
{
    Normal,
    Delicate
};

// Указує напрямок обертання пральної машинки
enum SpinDirection
{
    Clockwise,
    Counterclockwise
};

// Структура для поєднання номера обмотки з напрямком обертання
struct CoilDirection
{
    uint8_t coil;
    enum SpinDirection direction;
};

// Переходить до наступної обмотки, враховуючи поточний напрямок обертання
uint8_t nextCoil(struct CoilDirection coilDirection);

// Змінює напрямок обертання
enum SpinDirection changeSpinDirection(enum SpinDirection direction);

// Змінює режим прання
enum WashingMode changeWashingMode(enum WashingMode mode);

int main(void)
{

    DDRB = 0xF0;
    PORTB = ENGINE_BUTTON | MODE_BUTTON;

    // Кількість імпільсів, щоб повернути машинку на половину
    const uint16_t stepsPerHalf = STEPS_PER_SPIN * 0.5;

    // Кількість імпільсів, щоб повернути машинку на чверть
    const uint16_t stepsPerQuarter = STEPS_PER_SPIN * 0.25;

    // За замовчуванням нормальний режим
    enum WashingMode mode = Normal;

    bool engineButtonHeld = false;
    bool modeButtonHeld = false;

    bool engineOn = false; // машинку вимкнено

    uint8_t delicateCounter = 0; // до 8
    uint8_t normalCounter = 0;   // до 15

    // Початковий номер обмотки
    uint8_t coil = 0;

    // Напрямок за замовчування - за годинниковою стілкою
    enum SpinDirection direction = Clockwise;

    // Чи завершено прання ?
    bool washingFinished = true;

    while (true)
    {
        if (engineOn)
        {

            // перевірка вимикання двигуна
            if (PINB & ENGINE_BUTTON) // натиснуто кнопку вмикання / вимикання
            {
                if (engineButtonHeld)
                {
                    engineButtonHeld = false;

                    engineOn = false;
                }
                else
                {
                    engineButtonHeld = true;
#ifndef SIMULATION
                    _delay_ms(500);
#endif
                }
            }

            switch (mode)
            {

            case Normal: // Для нормального режиму

                if (normalCounter < NORMAL_TURNS)
                {
                    for (int i = 0; i < stepsPerHalf; i++)
                    {
                        PORTB = ((1 << coil) << 4) | ENGINE_BUTTON | MODE_BUTTON;

#ifndef SIMULATION

                        _delay_ms(SMALL_DELAY);
#endif
                        struct CoilDirection coilDirection;

                        coilDirection.coil = coil;
                        coilDirection.direction = direction;

                        coil = nextCoil(coilDirection);
                    }

                    ++normalCounter;
                    direction = changeSpinDirection(direction);
                }
                else
                    washingFinished = true;
                break;

            case Delicate: // Для делікатного

                if (delicateCounter < DELICATE_TURNS)
                {
                    for (int i = 0; i < stepsPerQuarter; i++)
                    {
                        PORTB = ((1 << coil) << 4) | ENGINE_BUTTON | MODE_BUTTON;
#ifndef SIMULATION
                        _delay_ms(BIG_DELAY);
#endif
                        struct CoilDirection coilDirection;

                        coilDirection.coil = coil;
                        coilDirection.direction = direction;

                        coil = nextCoil(coilDirection);
                    }

                    ++delicateCounter;

                    direction = changeSpinDirection(direction);
                }
                else
                    washingFinished = true;

                break;
            }

            if (washingFinished) // перевіряємо стан прання
            {
                engineOn = false;                    // вимикаємо двигун
                coil = 0;                            // скидаємо номер обмотки
                direction = Clockwise;               // скидаємо напрямок до значення за замовчуванням
                delicateCounter = 0;                 // скидаємо лічильник для делікатного режиму
                normalCounter = 0;                   // скидаємо лічильник для нормального режиму
                PORTB = ENGINE_BUTTON | MODE_BUTTON; // значення порту B за замовчуванням
            }
        }

        else
        {
            if (PINB & MODE_BUTTON) // натиснуто кнопку зміни режиму
            {
                if (modeButtonHeld) // чи кнопка зміни режиму досі утримується?
                {
                    modeButtonHeld = false;         // скасовуємо утримання
                    mode = changeWashingMode(mode); // змінюємо режим
                }
                else
                {
                    modeButtonHeld = true; // позначаємо початок утримання
#ifndef SIMULATION
                    _delay_ms(500); // півсекунди очікуємо
#endif
                }
            }

            if (PINB & ENGINE_BUTTON) // натиснуто кнопку вмикання / вимикання
            {
                if (engineButtonHeld)
                {
                    engineButtonHeld = false;

                    engineOn = true;
                }
                else
                {
                    engineButtonHeld = true;
#ifndef SIMULATION
                    _delay_ms(500);
#endif
                }
            }
        }
    }
}

uint8_t nextCoil(struct CoilDirection coilDirection)
{
    switch (coilDirection.direction)
    {
    case Clockwise:

        if (coilDirection.coil < (COILS_NUMBER - 1))
        {
            ++coilDirection.coil;
            return coilDirection.coil;
        }
        else
            return 0;

        break;

    case Counterclockwise:

        if (coilDirection.coil > 1)
        {
            --coilDirection.coil;
            return coilDirection.coil;
        }
        else
            return COILS_NUMBER - 1;

        break;
    }
}

enum SpinDirection changeSpinDirection(enum SpinDirection direction)
{
    switch (direction)
    {
    case Clockwise:
        direction = Counterclockwise;
        break;

    case Counterclockwise:
        direction = Clockwise;
        break;
    }

    return direction;
}

enum WashingMode changeWashingMode(enum WashingMode mode)
{
    if (mode == Normal)
        mode = Delicate;
    else if (mode == Delicate)
        mode = Normal;

    return mode;
}