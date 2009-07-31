/**************************************************************************
    Lightspark, a free flash player implementation

    Copyright (C) 2009  Alessandro Pignotti (a.pignotti@sssup.it)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#include <string>

#include "flashevents.h"
#include "swf.h"

using namespace std;

extern __thread SystemState* sys;

Event::Event(const string& t):type(t)
{
	setVariableByName("ENTER_FRAME",new ASString("enterFrame"));
	setVariableByName("ADDED_TO_STAGE",new ASString("addedToStage"));
	setVariableByName("INIT",new ASString("init"));
	setVariableByName("ADDED",new ASString("added"));
	setVariableByName("REMOVED",new ASString("removed"));
	setVariableByName("UNLOAD",new ASString("unload"));
	setVariableByName("ACTIVATE",new ASString("activate"));
	setVariableByName("DEACTIVATE",new ASString("deactivate"));
}

FocusEvent::FocusEvent():Event("focusEvent")
{
	setVariableByName("FOCUS_IN",new ASString("focusIn"));
	setVariableByName("FOCUS_OUT",new ASString("focusOut"));
	setVariableByName("MOUSE_FOCUS_CHANGE",new ASString("mouseFocusChange"));
	setVariableByName("KEY_FOCUS_CHANGE",new ASString("keyFocusChange"));
}

KeyboardEvent::KeyboardEvent():Event("keyboardEvent")
{
	setVariableByName("KEY_DOWN",new ASString("keyDown"));
	setVariableByName("KEY_UP",new ASString("keyUp"));
}

MouseEvent::MouseEvent():Event("mouseEvent")
{
	setVariableByName("MOUSE_DOWN",new ASString("mouseDown"));
	setVariableByName("MOUSE_UP",new ASString("mouseUp"));
	setVariableByName("CLICK",new ASString("click"));
}

IOErrorEvent::IOErrorEvent():Event("IOErrorEvent")
{
	setVariableByName("IO_ERROR",new ASString("ioError"));
}

EventDispatcher::EventDispatcher():id(0)
{
	if(constructor)
		constructor->decRef();
	constructor=new Function(_constructor);
}

ASFUNCTIONBODY(EventDispatcher,addEventListener)
{
	if(args->at(0)->getObjectType()!=T_STRING || args->at(1)->getObjectType()!=T_FUNCTION)
	{
		LOG(ERROR,"Type mismatch");
		abort();
	}
	EventDispatcher* th=dynamic_cast<EventDispatcher*>(obj);
	if(th==NULL)
		return NULL;
	sys->cur_input_thread->addListener(args->at(0)->toString(),th);

	IFunction* f=args->at(1)->toFunction();
	IFunction* f2=static_cast<IFunction*>(f->clone());
	f2->bind();

	th->handlers.insert(make_pair(args->at(0)->toString(),f2->toFunction()));
	sys->events_name.push_back(args->at(0)->toString());
}


ASFUNCTIONBODY(EventDispatcher,_constructor)
{
	obj->setVariableByName("addEventListener",new Function(addEventListener));
}

void EventDispatcher::handleEvent(Event* e)
{
	map<string, IFunction*>::iterator h=handlers.find(e->type);
	if(h==handlers.end())
	{
		LOG(NOT_IMPLEMENTED,"Not handled event");
		return;
	}

	LOG(CALLS, "Handling event " << h->first);
	arguments args(1);
	args.set(0,e);
	h->second->call(this,&args);
}

