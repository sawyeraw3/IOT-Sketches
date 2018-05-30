enum heartStep {One, Two, Three}; //, Four};
heartStep cStep = One;
bool heartOn = false;
unsigned long heartStart = 0;
unsigned long heartShiftDelay = 500;
void updateHeart(heartStep hStep);
