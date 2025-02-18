float get_duty_cycle(uint16_t value)//função que calcula o duty cycle
{
    if(value > 2150)
    {

        return (value - 2047) * 2;
    }
    else if(value < 1800)
    {
        return (2047 - value) * 2;
    }
    else
    {
        return 0;
    }
}