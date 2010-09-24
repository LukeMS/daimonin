/*-----------------------------------------------------------------------------
This source file is part of Daimonin's 3d-Client
Daimonin is a MMORG. Details can be found at http://daimonin.sourceforge.net
Copyright (c) 2005 Andreas Seidel

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

In addition, as a special exception, the copyright holder of client3d give
you permission to combine the client3d program with lgpl libraries of your
choice. You may copy and distribute such a system following the terms of the
GNU GPL for 3d-Client and the licenses of the other code concerned.

You should have received a copy of the GNU General Public License along with
this program; If not, see <http://www.gnu.org/licenses/>.
-----------------------------------------------------------------------------*/

#ifndef OBJECT_ELEMENT_AVATAR_H
#define OBJECT_ELEMENT_AVATAR_H

#include "object/object.h"
//#include "item.h"
//#define INVITEMBELOWXLEN 8
//#define INVITEMBELOWYLEN 1
//#define INVITEMXLEN 7
//#define INVITEMYLEN 3

/// @brief This class handles the avatar specific functions.
/// @details
class ObjectElementAvatar  : public ObjectElement
{
public:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    /// @brief Default constructor.
    ObjectElementAvatar(Object *parent);

    /// @brief Default destructor.
    ~ObjectElementAvatar();

    /// @brief Get the familyID of the element.
    Object::familyID getFamilyID() const
    {
        return Object::FAMILY_ANIMATE3D;
    }

    /// @brief Update this element.
    /// @param event Ogre frame event. Used to get time since last frame.
    bool update(const Ogre::FrameEvent &event);

private:
    // ////////////////////////////////////////////////////////////////////
    // Variables / Constants.
    // ////////////////////////////////////////////////////////////////////
    enum {MAX_SKILL = 6};
    typedef enum _attacks
    {
        /// We start with the double used attacks - for resist & protection too.
        /// damage type: physical
        ATNR_PHYSICAL, /**< = impact **/
        ATNR_SLASH,
        ATNR_CLEAVE,
        ATNR_PIERCE,
        /// damage type: elemental.
        ATNR_FIRE,
        ATNR_COLD,
        ATNR_ELECTRICITY,
        ATNR_POISON,
        ATNR_ACID,
        ATNR_SONIC,
        /// damage type: magical.
        ATNR_FORCE,
        ATNR_PSIONIC,
        ATNR_LIGHT,
        ATNR_SHADOW,
        ATNR_LIFESTEAL,
        /// damage type: sphere.
        ATNR_AETHER,
        ATNR_NETHER,
        ATNR_CHAOS,
        ATNR_DEATH,
        /// damage: type only effect by invulnerable.
        ATNR_WEAPONMAGIC,
        ATNR_GODPOWER,
        /// at this point attack effects starts - only resist maps to it
        ATNR_DRAIN,
        ATNR_DEPLETION,
        ATNR_CORRUPTION,
        ATNR_COUNTERMAGIC,
        ATNR_CANCELLATION,
        ATNR_CONFUSION,
        ATNR_FEAR,
        ATNR_SLOW,
        ATNR_PARALYZE,
        ATNR_SNARE,
        /// and the real special one here.
        ATNR_INTERNAL,
        SUM_ATTACKS
    } _attacks;

    typedef enum rangetype
    {
        range_bottom        =-1,
        range_none          = 0,
        range_bow           = 1,
        range_magic         = 2,
        range_wand          = 3,
        range_rod           = 4,
        range_scroll        = 5,
        range_horn          = 6,
        range_steal         = 7,
        range_size          = 8
    } rangetype;

    typedef struct
    {
        int  Str, Dex, Con, Wis, Cha, Int, Pow;
        int  wc;          /**< Weapon class **/
        int  ac;          /**< Armour Class **/
        int  level;
        int  hp;          /**< Hit Points (life). **/
        int  maxhp;
        int  sp;          /**< Spell points.  Used to cast spells. **/
        int  maxsp;       /**< Max spell points. **/
        int  grace;       /**< Grace points.  Used for prayers. */
        int  maxgrace;    /**< Max grace points. **/
        int  exp_level;
        int  exp;           /**< Experience **/
        int  food;          /**< How much food in stomach. 0 = starved. **/
        int  dam;           /**< How much damage this object does when hitting **/
        int  speed;         /**< Walking speed. **/
        float weapon_sp;    /**< Weapon speed. **/
        unsigned int flags;       /**< contains fire on/run on flags **/
        bool   protection_change; /**< Resistant value has changed **/
        short  protection[SUM_ATTACKS]; /**< Resistant values **/
        short  skill_level[MAX_SKILL];  /**< Level totals for skills **/
        int    skill_exp[MAX_SKILL];    /**< Experience totals for skills **/
    }
    Stats;

    // ////////////////////////////////////////////////////////////////////
    // Functions.
    // ////////////////////////////////////////////////////////////////////
    ObjectElementAvatar(const ObjectElementAvatar&);            ///< disable copy-constructor.
    ObjectElementAvatar &operator=(const ObjectElementAvatar&); ///< disable assignment operator.
};

#endif
