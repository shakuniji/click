/*
 * iprwpatterns.{cc,hh} -- stores shared IPRewriter patterns
 * Eddie Kohler
 *
 * Copyright (c) 2000 Massachusetts Institute of Technology.
 *
 * This software is being provided by the copyright holders under the GNU
 * General Public License, either version 2 or, at your discretion, any later
 * version. For more information, see the `COPYRIGHT' file in the source
 * distribution.
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif
#include "iprwpatterns.hh"
#include "confparse.hh"
#include "router.hh"
#include "error.hh"

IPRewriterPatterns::IPRewriterPatterns()
  : _name_map(-1)
{
}

int
IPRewriterPatterns::configure(const Vector<String> &conf, ErrorHandler *errh)
{
  // check for an earlier IPRewriterPatterns
  const Vector<Element *> &ev = router()->elements();
  for (int i = 0; i < eindex(); i++)
    if (IPRewriterPatterns *rwp = (IPRewriterPatterns *)ev[i]->cast("IPRewriterPatterns"))
      return rwp->configure(conf, errh);

  for (int i = 0; i < conf.size(); i++) {
    String word, rest;
    if (!cp_word(conf[i], &word, &rest)) {
      errh->error("pattern %d empty", i);
      continue;
    }
    cp_eat_space(rest);

    if (_name_map[word] >= 0) {
      errh->error("pattern name `%s' has already been defined", word.cc());
      continue;
    }

    IPRw::Pattern *p;
    if (IPRw::Pattern::parse(rest, &p, this, errh) >= 0) {
      p->use();
      _name_map.insert(word, _patterns.size());
      _patterns.push_back(p);
    }
  }
  return 0;
}

void
IPRewriterPatterns::uninitialize()
{
  for (int i = 0; i < _patterns.size(); i++)
    _patterns[i]->unuse();
}

IPRw::Pattern *
IPRewriterPatterns::find(Element *e, const String &name, ErrorHandler *errh)
{
  const Vector<Element *> &ev = e->router()->elements();
  for (int i = 0; i < ev.size(); i++)
    if (IPRewriterPatterns *rwp = (IPRewriterPatterns *)ev[i]->cast("IPRewriterPatterns")) {
      int x = rwp->_name_map[name];
      if (x >= 0)
	return rwp->_patterns[x];
      break;
    }
  errh->error("no pattern named `%s'", String(name).cc());
  return 0;
}

ELEMENT_REQUIRES(IPRw)
EXPORT_ELEMENT(IPRewriterPatterns)
