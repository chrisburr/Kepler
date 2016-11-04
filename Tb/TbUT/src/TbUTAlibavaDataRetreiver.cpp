/*
 * TbUTAlibava.cpp
 *
 *  Created on: Oct 1, 2014
 *      Author: ADendek
 */
#include "TbUTAlibavaDataRetreiver.h"

using namespace TbUT;

AlibavaDataRetreiver::AlibavaDataRetreiver () :
    m_assciRoot ()
{
}

void AlibavaDataRetreiver::open(std::string & p_filePath)
{
	m_assciRoot.open(p_filePath.c_str());
}

bool
AlibavaDataRetreiver::valid ()
{
  return m_assciRoot.valid ();
}

int AlibavaDataRetreiver::read_event (std::string & error_code)
{
  return m_assciRoot.read_event (error_code);
}

void AlibavaDataRetreiver::process_event ()
{
  m_assciRoot.process_event ();
}

double AlibavaDataRetreiver::time ()
{
  return m_assciRoot.time ();
}

double AlibavaDataRetreiver::temp ()
{
  return m_assciRoot.temp ();
}

unsigned short AlibavaDataRetreiver::data (int i)
{
  return m_assciRoot.data (i);
}
