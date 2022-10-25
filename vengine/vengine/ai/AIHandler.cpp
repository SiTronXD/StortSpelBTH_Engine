#include "AIHandler.hpp"

void AIHandler::drawImgui(){

    
    for(auto fsmImguiLambda : FSMimguiLambdas)
    {
        fsmImguiLambda.second(fsmImguiLambda.first);
    }    
}