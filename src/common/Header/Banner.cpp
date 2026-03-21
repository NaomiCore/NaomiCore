/*
 * This file is part of the NaomiCore Project. Powered by GetRight 2026
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Banner.h"
#include "GitRevision.h"
#include "StringFormat.h"


void Trinity::Banner::Show(char const* applicationName, void(*log)(char const* text), void(*logExtraInfo)())
{
	log(R"( >> Powered by GetRight 2026 )");
    log(Trinity::StringFormat("{} ({})", GitRevision::GetFullVersion(), applicationName).c_str());
	log(R"( )"); 
    log(R"(     _   __    TIME TO GAME      _ ______    Version 3    )");      
    log(R"(    / | / /___ _____  ____ ___  (_) ____/___  ________    )");
    log(R"(   /  |/ / __ `/ __ \/ __ `__ \/ / /   / __ \/ ___/ _ \   )");
    log(R"(  / /|  / /_/ / /_/ / / / / / / / /___/ /_/ / /  /  __/   )");
    log(R"( /_/ |_/\__,_/\____/_/ /_/ /_/_/\____/\____/_/   \___/    )");
	log(R"(                                                          )");
    log(R"(    ________________                      Free Software   )");
    log(R"( __/   Information  \___________________________________________ )");
	log(R"(                                                                   )");
	log(R"( - Wrath of The Lich King 3      ›     - Assembly - 20.03.2026)"); 
	log(R"( - Blizzlike Wow - Server        ›     - Build - 3.6.2 - UC3 )"); 
	log(R"( - Mods used in the assembly     ›     - SoloLFG and FIX Core )");  
	log(R"( - Mail: Forsemark@gmail.com     ›     - Database: UCDB 3.3.5a)" "\n");
    log(R"( )");
	
    if (logExtraInfo)
        logExtraInfo();
}
