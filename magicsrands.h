/*
-----------------------------------------------------------------------------
This source file is part of the Havoc chess engine
Copyright (c) 2020 Minniesoft
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/
#pragma once

#include "types.h"
namespace {


  std::vector<U64> bishop_magics = 
  {
    static_cast<U64>(1143494273958400), static_cast<U64>(1155182134877503488), static_cast<U64>(4507998752866305), static_cast<U64>(1143775829164096), static_cast<U64>(432644906268229633), static_cast<U64>(143006304830720), static_cast<U64>(36173950270832640),
    static_cast<U64>(18176369755136), static_cast<U64>(316676545970432), static_cast<U64>(2269409188020736), static_cast<U64>(288247985526026240), static_cast<U64>(30872233312256), static_cast<U64>(18018848632471568),
    static_cast<U64>(9259401418526687232), static_cast<U64>(551971471362), static_cast<U64>(2203326779392), static_cast<U64>(18296010992517248), static_cast<U64>(38280614147260480), static_cast<U64>(4503634526085152),
    static_cast<U64>(2251801994739976), static_cast<U64>(563018975936528), static_cast<U64>(281483570971136), static_cast<U64>(9288682829914112), static_cast<U64>(140742353817600), static_cast<U64>(4573968506290433),
    static_cast<U64>(2256232355201152), static_cast<U64>(52776591951936), static_cast<U64>(4620702013791944712), static_cast<U64>(2306406096639656000), static_cast<U64>(4616189892935028736), static_cast<U64>(282578850676992),
    static_cast<U64>(72199431046318080), static_cast<U64>(1143784151195648), static_cast<U64>(149671020594304), static_cast<U64>(1153484660718829696), static_cast<U64>(288265698500149376), static_cast<U64>(1126183374946568),
    static_cast<U64>(9011598375094272), static_cast<U64>(4507998747656704), static_cast<U64>(145146272350336), static_cast<U64>(36609348285237248), static_cast<U64>(290275633660928), static_cast<U64>(17867869263872),
    static_cast<U64>(9079695085696), static_cast<U64>(2305851814032966656), static_cast<U64>(4504767875252512), static_cast<U64>(565183353192960), static_cast<U64>(1134698149446912), static_cast<U64>(1163317729296384),
    static_cast<U64>(141322695475200), static_cast<U64>(140879357018112), static_cast<U64>(4503617914929152), static_cast<U64>(216172851940687872), static_cast<U64>(54052060609413120), static_cast<U64>(290499785465544704),
    static_cast<U64>(2260600206000136), static_cast<U64>(140877109411840), static_cast<U64>(1126466909701120), static_cast<U64>(547622928), static_cast<U64>(21107202), static_cast<U64>(18014398780023298),
    static_cast<U64>(2252074825876736), static_cast<U64>(4466833687552), static_cast<U64>(1130366677155968)
  };

  std::vector<U64> rook_magics = 
  {
    static_cast<U64>(36029347045326849), static_cast<U64>(18016881537454080), static_cast<U64>(36037730551989376), static_cast<U64>(36037593380423296), static_cast<U64>(72062065099016192), static_cast<U64>(36030998189769728), static_cast<U64>(4647785733963121152),
    static_cast<U64>(36029072442132736), static_cast<U64>(140738562631808), static_cast<U64>(4899987038203285504), static_cast<U64>(4611827305940000768), static_cast<U64>(2305984296726317056), static_cast<U64>(141029554538496),
    static_cast<U64>(36169551687319681), static_cast<U64>(36169676249629184), static_cast<U64>(36169536654821632), static_cast<U64>(4539333759468672), static_cast<U64>(2314850484151664640), static_cast<U64>(282574769364992),
    static_cast<U64>(142386890018816), static_cast<U64>(2306969459010600964), static_cast<U64>(144397762631305218), static_cast<U64>(1154188691774636544), static_cast<U64>(142936520261697), static_cast<U64>(74379764743307296),
    static_cast<U64>(9024792514535496), static_cast<U64>(4620702014849941568), static_cast<U64>(8798247846016), static_cast<U64>(4512397868138880), static_cast<U64>(2341876206435041792), static_cast<U64>(565166156615688),
    static_cast<U64>(140739636380416), static_cast<U64>(18014948810559552), static_cast<U64>(36169809410400256), static_cast<U64>(2955555983335424), static_cast<U64>(598203061766400), static_cast<U64>(4512397893043200),
    static_cast<U64>(2306265238867018240), static_cast<U64>(1153062521276465664), static_cast<U64>(4399153809537), static_cast<U64>(140876001083410), static_cast<U64>(37225066206888064), static_cast<U64>(36310341252087872),
    static_cast<U64>(862017250427008), static_cast<U64>(567348134183040), static_cast<U64>(577028100336943232), static_cast<U64>(3378799265742976), static_cast<U64>(141014530654212), static_cast<U64>(288371123303751808),
    static_cast<U64>(4503874658369600), static_cast<U64>(70506184245504), static_cast<U64>(2542073031950464), static_cast<U64>(1134696067006592), static_cast<U64>(141991618936960), static_cast<U64>(140746080452736),
    static_cast<U64>(2392538392560000), static_cast<U64>(17871361048577), static_cast<U64>(70373040201857), static_cast<U64>(281750391490577), static_cast<U64>(17592723505157), static_cast<U64>(563019210294274),
    static_cast<U64>(281492156745729), static_cast<U64>(562952168081410), static_cast<U64>(1101661225986)
  };
};
