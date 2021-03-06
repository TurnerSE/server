#include "platform.h"
#include "config.h"
#include "types.h"
#include "ally.h"

#include "save.h"
#include "unit.h"
#include "region.h"
#include "group.h"
#include "faction.h"
#include "plane.h"

#include <util/attrib.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>

ally * ally_find(ally *al, const struct faction *f) {
    for (; al; al = al->next) {
        if (al->faction == f) return al;
    }
    return 0;
}

ally * ally_add(ally **al_p, struct faction *f) {
    ally * al;
    while (*al_p) {
        al = *al_p;
        if (f && al->faction == f) return al;
        al_p = &al->next;
    }
    al = (ally *)malloc(sizeof(ally));
    al->faction = f;
    al->status = 0;
    al->next = 0;
    *al_p = al;
    return al;
}

void ally_remove(ally **al_p, struct faction *f) {
    ally * al;
    while (*al_p) {
        al = *al_p;
        if (al->faction == f) {
            *al_p = al->next;
            free(al);
            break;
        }
        al_p = &al->next;
    }
}

static int ally_flag(const char *s, int help_mask)
{
    if ((help_mask & HELP_MONEY) && strcmp(s, "money") == 0)
        return HELP_MONEY;
    if ((help_mask & HELP_FIGHT) && strcmp(s, "fight") == 0)
        return HELP_FIGHT;
    if ((help_mask & HELP_GIVE) && strcmp(s, "give") == 0)
        return HELP_GIVE;
    if ((help_mask & HELP_GUARD) && strcmp(s, "guard") == 0)
        return HELP_GUARD;
    if ((help_mask & HELP_FSTEALTH) && strcmp(s, "stealth") == 0)
        return HELP_FSTEALTH;
    if ((help_mask & HELP_TRAVEL) && strcmp(s, "travel") == 0)
        return HELP_TRAVEL;
    return 0;
}

/** Specifies automatic alliance modes.
* If this returns a value then the bits set are immutable between alliance
* partners (faction::alliance) and cannot be changed with the HELP command.
*/
int AllianceAuto(void)
{
    int value;
    const char *str = config_get("alliance.auto");
    value = 0;
    if (str != NULL) {
        char *sstr = _strdup(str);
        char *tok = strtok(sstr, " ");
        while (tok) {
            value |= ally_flag(tok, -1);
            tok = strtok(NULL, " ");
        }
        free(sstr);
    }
    return value & HelpMask();
}

static int
autoalliance(const plane * pl, const faction * sf, const faction * f2)
{
    if (pl && (pl->flags & PFL_FRIENDLY))
        return HELP_ALL;

    if (f_get_alliance(sf) != NULL && AllianceAuto()) {
        if (sf->alliance == f2->alliance)
            return AllianceAuto();
    }

    return 0;
}

static int ally_mode(const ally * sf, int mode)
{
    if (sf == NULL)
        return 0;
    return sf->status & mode;
}

static void init_npcfaction(struct attrib *a)
{
    a->data.i = 1;
}

attrib_type at_npcfaction = {
    "npcfaction",
    init_npcfaction,
    NULL,
    NULL,
    a_writeint,
    a_readint,
    ATF_UNIQUE
};

/** Limits the available help modes
* The bitfield returned by this function specifies the available help modes
* in this game (so you can, for example, disable HELP GIVE globally).
* Disabling a status will disable the command sequence entirely (order parsing
* uses this function).
*/
int HelpMask(void)
{
    const char *str = config_get("rules.help.mask");
    int rule = 0;
    if (str != NULL) {
        char *sstr = _strdup(str);
        char *tok = strtok(sstr, " ");
        while (tok) {
            rule |= ally_flag(tok, -1);
            tok = strtok(NULL, " ");
        }
        free(sstr);
    }
    else {
        rule = HELP_ALL;
    }
    return rule;
}

static int AllianceRestricted(void)
{
    const char *str = config_get("alliance.restricted");
    int rule = 0;
    if (str != NULL) {
        char *sstr = _strdup(str);
        char *tok = strtok(sstr, " ");
        while (tok) {
            rule |= ally_flag(tok, -1);
            tok = strtok(NULL, " ");
        }
        free(sstr);
    }
    rule &= HelpMask();
    return rule;
}

int
alliedgroup(const struct plane *pl, const struct faction *f,
    const struct faction *f2, const struct ally *sf, int mode)
{
    while (sf && sf->faction != f2)
        sf = sf->next;
    if (sf == NULL) {
        mode = mode & autoalliance(pl, f, f2);
    }
    mode = ally_mode(sf, mode) | (mode & autoalliance(pl, f, f2));
    if (AllianceRestricted()) {
        if (a_findc(f->attribs, &at_npcfaction)) {
            return mode;
        }
        if (a_findc(f2->attribs, &at_npcfaction)) {
            return mode;
        }
        if (f->alliance != f2->alliance) {
            mode &= ~AllianceRestricted();
        }
    }
    return mode;
}

int
alliedfaction(const struct plane *pl, const struct faction *f,
    const struct faction *f2, int mode)
{
    return alliedgroup(pl, f, f2, f->allies, mode);
}

/* Die Gruppe von Einheit u hat helfe zu f2 gesetzt. */
int alliedunit(const unit * u, const faction * f2, int mode)
{
    ally *sf;
    int automode;

    assert(u);
    assert(f2);
    assert(u->region);            /* the unit should be in a region, but it's possible that u->number==0 (TEMP units) */
    if (u->faction == f2)
        return mode;
    if (u->faction != NULL && f2 != NULL) {
        plane *pl;

        if (mode & HELP_FIGHT) {
            if ((u->flags & UFL_DEFENDER) || (u->faction->flags & FFL_DEFENDER)) {
                faction *owner = region_get_owner(u->region);
                /* helps the owner of the region */
                if (owner == f2) {
                    return HELP_FIGHT;
                }
            }
        }

        pl = rplane(u->region);
        automode = mode & autoalliance(pl, u->faction, f2);

        if (pl != NULL && (pl->flags & PFL_NOALLIANCES))
            mode = (mode & automode) | (mode & HELP_GIVE);

        sf = u->faction->allies;
        if (fval(u, UFL_GROUP)) {
            const attrib *a = a_findc(u->attribs, &at_group);
            if (a != NULL)
                sf = ((group *)a->data.v)->allies;
        }
        return alliedgroup(pl, u->faction, f2, sf, mode);
    }
    return 0;
}

