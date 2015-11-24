/** optiMEAS GmbH Measurement and Automation Systems
 *  ------------------------------------------------
 *
 *  copright (c) 2015 -
 *  author:      $Author$
 *
 *  e-Mail:      info@optimeas.de
 *  web:         http://www.optimeas.de
 *
 *  This file is part of the project:
 *
 *      $Project$
 *      ---------------------------------------------
 *
 * *************************************************************************
 *
 * The source code is given as is. The author is not responsible for any
 * possible damage done due to the use of this code. The component can be
 * freely used in any application. Source code remains property of the
 * author and may not be distributed, published, given or sold in any
 * form as such. No parts of the source code can be included in any other
 * component or application without written authorization of optiMEAS GmbH.
 *
 * The documents version, history and changelog is provided from the
 * version control system (SVN, GIT, etc.). It is never added to the
 * document itself avoiding unnecessary changes and conflicts during
 * merge.
 *
 * ************************************************************************
 */

#include <om_cpp_utils/filesystem.hpp>

#include <fstream>

namespace cu {

bool OM_CPP_UTILS_API fileExist(const std::string &fileName)
{
    std::ifstream infile(fileName);
    return infile.good();
}

} // namespace cu
