// Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Ensures each implementation of HighwayHash returns consistent and unchanging
// hash values.

#include "highwayhash_test_target.h"

#include <stddef.h>
#include <atomic>
#include <cstdio>
#include <cstdlib>

#ifdef HH_GOOGLETEST
#include "testing/base/public/gunit.h"
#endif

#include "data_parallel.h"
#include "highwayhash_target.h"
#include "instruction_sets.h"

// Define to nonzero in order to print the (new) golden outputs.
// WARNING: HighwayHash is frozen, so the golden values must not change.
#define PRINT_RESULTS 0

namespace highwayhash {
namespace {

// Known-good outputs are verified for all lengths in [0, 64].
const size_t kMaxSize = 64;

#if PRINT_RESULTS
void Print(const HHResult64 result) { printf("0x%016lXull,\n", result); }

// For HHResult128/256.
template <int kNumLanes>
void Print(const HHResult64 (&result)[kNumLanes]) {
  printf("{ ");
  for (int i = 0; i < kNumLanes; ++i) {
    if (i != 0) {
      printf(", ");
    }
    printf("0x%016lXull", result[i]);
  }
  printf("},\n");
}
#endif  // PRINT_RESULTS

// Called when any test fails; exits immediately because one mismatch usually
// implies many others.
void OnFailure(const char* target_name, const size_t size) {
  printf("Mismatch at size %zu for target %s\n", size, target_name);
#ifdef HH_GOOGLETEST
  EXPECT_TRUE(false);
#endif
  exit(1);
}

// Verifies every combination of implementation and input size. Returns which
// targets were run/verified.
template <typename Result>
TargetBits VerifyImplementations(const Result (&known_good)[kMaxSize + 1]) {
  const HHKey key = {0x0706050403020100ULL, 0x0F0E0D0C0B0A0908ULL,
                     0x1716151413121110ULL, 0x1F1E1D1C1B1A1918ULL};

  TargetBits targets = ~0U;

  // For each test input: empty string, 00, 00 01, ...
  char in[kMaxSize + 1] = {0};
  // Fast enough that we don't need a thread pool.
  for (uint64_t size = 0; size <= kMaxSize; ++size) {
    in[size] = static_cast<char>(size);
#if PRINT_RESULTS
    Result actual;
    targets &= InstructionSets::Run<HighwayHash>(key, in, size, &actual);
    Print(actual);
#else
    const Result* expected = &known_good[size];
    targets &= InstructionSets::RunAll<HighwayHashTest>(key, in, size, expected,
                                                        &OnFailure);
#endif
  }
  return targets;
}

// Cat

void OnCatFailure(const char* target_name, const size_t size) {
  printf("Cat mismatch at size %zu\n", size);
#ifdef HH_GOOGLETEST
  EXPECT_TRUE(false);
#endif
  exit(1);
}

// Returns which targets were run/verified.
template <typename Result>
TargetBits VerifyCat(ThreadPool* pool) {
  // Reversed order vs prior test.
  const HHKey key = {0x1F1E1D1C1B1A1918ULL, 0x1716151413121110ULL,
                     0x0F0E0D0C0B0A0908ULL, 0x0706050403020100ULL};

  const size_t kMaxSize = 3 * 35;
  char flat[kMaxSize];
  srand(129);
  for (size_t size = 0; size < kMaxSize; ++size) {
    flat[size] = static_cast<char>(rand() & 0xFF);
  }

  std::atomic<TargetBits> targets{~0U};

  pool->Run(0, kMaxSize, [&key, &flat, &targets](const uint32_t i) {
    Result dummy;
    targets.fetch_and(InstructionSets::RunAll<HighwayHashCatTest>(
        key, flat, i, &dummy, &OnCatFailure));
  });
  return targets.load();
}

// WARNING: HighwayHash is frozen, so the golden values must not change.
const HHResult64 kExpected64[kMaxSize + 1] = {
    0x907A56DE22C26E53ull, 0x7EAB43AAC7CDDD78ull, 0xB8D0569AB0B53D62ull,
    0x5C6BEFAB8A463D80ull, 0xF205A46893007EDAull, 0x2B8A1668E4A94541ull,
    0xBD4CCC325BEFCA6Full, 0x4D02AE1738F59482ull, 0xE1205108E55F3171ull,
    0x32D2644EC77A1584ull, 0xF6E10ACDB103A90Bull, 0xC3BBF4615B415C15ull,
    0x243CC2040063FA9Cull, 0xA89A58CE65E641FFull, 0x24B031A348455A23ull,
    0x40793F86A449F33Bull, 0xCFAB3489F97EB832ull, 0x19FE67D2C8C5C0E2ull,
    0x04DD90A69C565CC2ull, 0x75D9518E2371C504ull, 0x38AD9B1141D3DD16ull,
    0x0264432CCD8A70E0ull, 0xA9DB5A6288683390ull, 0xD7B05492003F028Cull,
    0x205F615AEA59E51Eull, 0xEEE0C89621052884ull, 0x1BFC1A93A7284F4Full,
    0x512175B5B70DA91Dull, 0xF71F8976A0A2C639ull, 0xAE093FEF1F84E3E7ull,
    0x22CA92B01161860Full, 0x9FC7007CCF035A68ull, 0xA0C964D9ECD580FCull,
    0x2C90F73CA03181FCull, 0x185CF84E5691EB9Eull, 0x4FC1F5EF2752AA9Bull,
    0xF5B7391A5E0A33EBull, 0xB9B84B83B4E96C9Cull, 0x5E42FE712A5CD9B4ull,
    0xA150F2F90C3F97DCull, 0x7FA522D75E2D637Dull, 0x181AD0CC0DFFD32Bull,
    0x3889ED981E854028ull, 0xFB4297E8C586EE2Dull, 0x6D064A45BB28059Cull,
    0x90563609B3EC860Cull, 0x7AA4FCE94097C666ull, 0x1326BAC06B911E08ull,
    0xB926168D2B154F34ull, 0x9919848945B1948Dull, 0xA2A98FC534825EBEull,
    0xE9809095213EF0B6ull, 0x582E5483707BC0E9ull, 0x086E9414A88A6AF5ull,
    0xEE86B98D20F6743Dull, 0xF89B7FF609B1C0A7ull, 0x4C7D9CC19E22C3E8ull,
    0x9A97005024562A6Full, 0x5DD41CF423E6EBEFull, 0xDF13609C0468E227ull,
    0x6E0DA4F64188155Aull, 0xB755BA4B50D7D4A1ull, 0x887A3484647479BDull,
    0xAB8EEBE9BF2139A0ull, 0x75542C5D4CD2A6FFull};

// WARNING: HighwayHash is frozen, so the golden values must not change.
const HHResult128 kExpected128[kMaxSize + 1] = {
    {0x0FED268F9D8FFEC7ull, 0x33565E767F093E6Full},
    {0xD6B0A8893681E7A8ull, 0xDC291DF9EB9CDCB4ull},
    {0x3D15AD265A16DA04ull, 0x78085638DC32E868ull},
    {0x0607621B295F0BEBull, 0xBFE69A0FD9CEDD79ull},
    {0x26399EB46DACE49Eull, 0x2E922AD039319208ull},
    {0x3250BDC386D12ED8ull, 0x193810906C63C23Aull},
    {0x6F476AB3CB896547ull, 0x7CDE576F37ED1019ull},
    {0x2A401FCA697171B4ull, 0xBE1F03FF9F02796Cull},
    {0xA1E96D84280552E8ull, 0x695CF1C63BEC0AC2ull},
    {0x142A2102F31E63B2ull, 0x1A85B98C5B5000CCull},
    {0x51A1B70E26B6BC5Bull, 0x929E1F3B2DA45559ull},
    {0x88990362059A415Bull, 0xBED21F22C47B7D13ull},
    {0xCD1F1F5F1CAF9566ull, 0xA818BA8CE0F9C8D4ull},
    {0xA225564112FE6157ull, 0xB2E94C78B8DDB848ull},
    {0xBD492FEBD1CC0919ull, 0xCECD1DBC025641A2ull},
    {0x142237A52BC4AF54ull, 0xE0796C0B6E26BCD7ull},
    {0x414460FFD5A401ADull, 0x029EA3D5019F18C8ull},
    {0xC52A4B96C51C9962ull, 0xECB878B1169B5EA0ull},
    {0xD940CA8F11FBEACEull, 0xF93A46D616F8D531ull},
    {0x8AC49D0AE5C0CBF5ull, 0x3FFDBF8DF51D7C93ull},
    {0xAC6D279B852D00A8ull, 0x7DCD3A6BA5EBAA46ull},
    {0xF11621BD93F08A56ull, 0x3173C398163DD9D5ull},
    {0x0C4CE250F68CF89Full, 0xB3123CDA411898EDull},
    {0x15AB97ED3D9A51CEull, 0x7CE274479169080Eull},
    {0xCD001E198D4845B8ull, 0xD0D9D98BD8AA2D77ull},
    {0x34F3D617A0493D79ull, 0x7DD304F6397F7E16ull},
    {0x5CB56890A9F4C6B6ull, 0x130829166567304Full},
    {0x30DA6F8B245BD1C0ull, 0x6F828B7E3FD9748Cull},
    {0xE0580349204C12C0ull, 0x93F6DA0CAC5F441Cull},
    {0xF648731BA5073045ull, 0x5FB897114FB65976ull},
    {0x024F8354738A5206ull, 0x509A4918EB7E0991ull},
    {0x06E7B465E8A57C29ull, 0x52415E3A07F5D446ull},
    {0x1984DF66C1434AAAull, 0x16FC1958F9B3E4B9ull},
    {0x111678AFE0C6C36Cull, 0xF958B59DE5A2849Dull},
    {0x773FBC8440FB0490ull, 0xC96ED5D243658536ull},
    {0x91E3DC710BB6C941ull, 0xEA336A0BC1EEACE9ull},
    {0x25CFE3815D7AD9D4ull, 0xF2E94F8C828FC59Eull},
    {0xB9FB38B83CC288F2ull, 0x7479C4C8F850EC04ull},
    {0x1D85D5C525982B8Cull, 0x6E26B1C16F48DBF4ull},
    {0x8A4E55BD6060BDE7ull, 0x2134D599058B3FD0ull},
    {0x2A958FF994778F36ull, 0xE8052D1AE61D6423ull},
    {0x89233AE6BE453233ull, 0x3ACF9C87D7E8C0B9ull},
    {0x4458F5E27EA9C8D5ull, 0x418FB49BCA2A5140ull},
    {0x090301837ED12A68ull, 0x1017F69633C861E6ull},
    {0x330DD84704D49590ull, 0x339DF1AD3A4BA6E4ull},
    {0x569363A663F2C576ull, 0x363B3D95E3C95EF6ull},
    {0xACC8D08586B90737ull, 0x2BA0E8087D4E28E9ull},
    {0x39C27A27C86D9520ull, 0x8DB620A45160932Eull},
    {0x8E6A4AEB671A072Dull, 0x6ED3561A10E47EE6ull},
    {0x0011D765B1BEC74Aull, 0xD80E6E656EDE842Eull},
    {0x2515D62B936AC64Cull, 0xCE088794D7088A7Dull},
    {0x91621552C16E23AFull, 0x264F0094EB23CCEFull},
    {0x1E21880D97263480ull, 0xD8654807D3A31086ull},
    {0x39D76AAF097F432Dull, 0xA517E1E09D074739ull},
    {0x0F17A4F337C65A14ull, 0x2F51215F69F976D4ull},
    {0xA0FB5CDA12895E44ull, 0x568C3DC4D1F13CD1ull},
    {0x93C8FC00D89C46CEull, 0xBAD5DA947E330E69ull},
    {0x817C07501D1A5694ull, 0x584D6EE72CBFAC2Bull},
    {0x91D668AF73F053BFull, 0xF98E647683C1E0EDull},
    {0x5281E1EF6B3CCF8Bull, 0xBC4CC3DF166083D8ull},
    {0xAAD61B6DBEAAEEB9ull, 0xFF969D000C16787Bull},
    {0x4325D84FC0475879ull, 0x14B919BD905F1C2Dull},
    {0x79A176D1AA6BA6D1ull, 0xF1F720C5A53A2B86ull},
    {0x74BD7018022F3EF0ull, 0x3AEA94A8AD5F4BCBull},
    {0x98BB1F7198D4C4F2ull, 0xE0BC0571DE918FC8ull}};

// WARNING: HighwayHash is frozen, so the golden values must not change.
const HHResult256 kExpected256[kMaxSize + 1] = {
    {0xDD44482AC2C874F5ull, 0xD946017313C7351Full, 0xB3AEBECCB98714FFull,
     0x41DA233145751DF4ull},
    {0xEDB941BCE45F8254ull, 0xE20D44EF3DCAC60Full, 0x72651B9BCB324A47ull,
     0x2073624CB275E484ull},
    {0x3FDFF9DF24AFE454ull, 0x11C4BF1A1B0AE873ull, 0x115169CC6922597Aull,
     0x1208F6590D33B42Cull},
    {0x480AA0D70DD1D95Cull, 0x89225E7C6911D1D0ull, 0x8EA8426B8BBB865Aull,
     0xE23DFBC390E1C722ull},
    {0xC9CFC497212BE4DCull, 0xA85F9DF6AFD2929Bull, 0x1FDA9F211DF4109Eull,
     0x07E4277A374D4F9Bull},
    {0xB4B4F566A4DC85B3ull, 0xBF4B63BA5E460142ull, 0x15F48E68CDDC1DE3ull,
     0x0F74587D388085C6ull},
    {0x6445C70A86ADB9B4ull, 0xA99CFB2784B4CEB6ull, 0xDAE29D40A0B2DB13ull,
     0xB6526DF29A9D1170ull},
    {0xD666B1A00987AD81ull, 0xA4F1F838EB8C6D37ull, 0xE9226E07D463E030ull,
     0x5754D67D062C526Cull},
    {0xF1B905B0ED768BC0ull, 0xE6976FF3FCFF3A45ull, 0x4FBE518DD9D09778ull,
     0xD9A0AFEB371E0D33ull},
    {0x80D8E4D70D3C2981ull, 0xF10FBBD16424F1A1ull, 0xCF5C2DBE9D3F0CD1ull,
     0xC0BFE8F701B673F2ull},
    {0xADE48C50E5A262BEull, 0x8E9492B1FDFE38E0ull, 0x0784B74B2FE9B838ull,
     0x0E41D574DB656DCDull},
    {0xA1BE77B9531807CFull, 0xBA97A7DE6A1A9738ull, 0xAF274CEF9C8E261Full,
     0x3E39B935C74CE8E8ull},
    {0x15AD3802E3405857ull, 0x9D11CBDC39E853A0ull, 0x23EA3E993C31B225ull,
     0x6CD9E9E3CAF4212Eull},
    {0x01C96F5EB1D77C36ull, 0xA367F9C1531F95A6ull, 0x1F94A3427CDADCB8ull,
     0x97F1000ABF3BD5D3ull},
    {0x0815E91EEEFF8E41ull, 0x0E0C28FA6E21DF5Dull, 0x4EAD8E62ED095374ull,
     0x3FFD01DA1C9D73E6ull},
    {0xC11905707842602Eull, 0x62C3DB018501B146ull, 0x85F5AD17FA3406C1ull,
     0xC884F87BD4FEC347ull},
    {0xF51AD989A1B6CD1Full, 0xF7F075D62A627BD9ull, 0x7E01D5F579F28A06ull,
     0x1AD415C16A174D9Full},
    {0x19F4CFA82CA4068Eull, 0x3B9D4ABD3A9275B9ull, 0x8000B0DDE9C010C6ull,
     0x8884D50949215613ull},
    {0x126D6C7F81AB9F5Dull, 0x4EDAA3C5097716EEull, 0xAF121573A7DD3E49ull,
     0x9001AC85AA80C32Dull},
    {0x06AABEF9149155FAull, 0xDF864F4144E71C3Dull, 0xFDBABCE860BC64DAull,
     0xDE2BA54792491CB6ull},
    {0xADFC6B4035079FDBull, 0xA087B7328E486E65ull, 0x46D1A9935A4623EAull,
     0xE3895C440D3CEE44ull},
    {0xB5F9D31DEEA3B3DFull, 0x8F3024E20A06E133ull, 0xF24C38C8288FE120ull,
     0x703F1DCF9BD69749ull},
    {0x2B3C0B854794EFE3ull, 0x1C5D3F969BDACEA0ull, 0x81F16AAFA563AC2Eull,
     0x23441C5A79D03075ull},
    {0x418AF8C793FD3762ull, 0xBC6B8E9461D7F924ull, 0x776FF26A2A1A9E78ull,
     0x3AA0B7BFD417CA6Eull},
    {0xCD03EA2AD255A3C1ull, 0x0185FEE5B59C1B2Aull, 0xD1F438D44F9773E4ull,
     0xBE69DD67F83B76E4ull},
    {0xF951A8873887A0FBull, 0x2C7B31D2A548E0AEull, 0x44803838B6186EFAull,
     0xA3C78EC7BE219F72ull},
    {0x958FF151EA0D8C08ull, 0x4B7E8997B4F63488ull, 0xC78E074351C5386Dull,
     0xD95577556F20EEFAull},
    {0x29A917807FB05406ull, 0x3318F884351F578Cull, 0xDD24EA6EF6F6A7FAull,
     0xE74393465E97AEFFull},
    {0x98240880935E6CCBull, 0x1FD0D271B09F97DAull, 0x56E786472700B183ull,
     0x291649F99F747817ull},
    {0x1BD4954F7054C556ull, 0xFFDB2EFF7C596CEBull, 0x7C6AC69A1BAB6B5Bull,
     0x0F037670537FC153ull},
    {0x8825E38897597498ull, 0x647CF6EBAF6332C1ull, 0x552BD903DC28C917ull,
     0x72D7632C00BFC5ABull},
    {0x6880E276601A644Dull, 0xB3728B20B10FB7DAull, 0xD0BD12060610D16Eull,
     0x8AEF14EF33452EF2ull},
    {0xBCE38C9039A1C3FEull, 0x42D56326A3C11289ull, 0xE35595F764FCAEA9ull,
     0xC9B03C6BC9475A99ull},
    {0xF60115CBF034A6E5ull, 0x6C36EA75BFCE46D0ull, 0x3B17C8D382725990ull,
     0x7EDAA2ED11007A35ull},
    {0x1326E959EDF9DEA2ull, 0xC4776801739F720Cull, 0x5169500FD762F62Full,
     0x8A0DD0D90A2529ABull},
    {0x935149D503D442D4ull, 0xFF6BB41302DAD144ull, 0x339CB012CD9D36ECull,
     0xE61D53619ECC2230ull},
    {0x528BC888AA50B696ull, 0xB8AEECA36084E1FCull, 0xA158151EC0243476ull,
     0x02C14AAD097CEC44ull},
    {0xBED688A72217C327ull, 0x1EE65114F760873Full, 0x3F5C26B37D3002A6ull,
     0xDDF2E895631597B9ull},
    {0xE7DB21CF2B0B51ADull, 0xFAFC6324F4B0AB6Cull, 0xB0857244C22D9C5Bull,
     0xF0AD888D1E05849Cull},
    {0x05519793CD4DCB00ull, 0x3C594A3163067DEBull, 0xAC75081ACF119E34ull,
     0x5AC86297805CB094ull},
    {0x09228D8C22B5779Eull, 0x19644DB2516B7E84ull, 0x2B92C8ABF83141A0ull,
     0x7F785AD725E19391ull},
    {0x59C42E5D46D0A74Bull, 0x5EA53C65CA036064ull, 0x48A9916BB635AEB4ull,
     0xBAE6DF143F54E9D4ull},
    {0x5EB623696D03D0E3ull, 0xD53D78BCB41DA092ull, 0xFE2348DC52F6B10Dull,
     0x64802457632C8C11ull},
    {0x43B61BB2C4B85481ull, 0xC6318C25717E80A1ull, 0x8C4A7F4D6F9C687Dull,
     0xBD0217E035401D7Cull},
    {0x7F51CA5743824C37ull, 0xB04C4D5EB11D703Aull, 0x4D511E1ECBF6F369ull,
     0xD66775EA215456E2ull},
    {0x39B409EEF87E45CCull, 0x52B8E8C459FC79B3ull, 0x44920918D1858C24ull,
     0x80F07B645EEE0149ull},
    {0xCE8694D1BE9AD514ull, 0xBFA19026526836E7ull, 0x1EA4FDF6E4902A7Dull,
     0x380C4458D696E1FEull},
    {0xD189E18BF823A0A4ull, 0x1F3B353BE501A7D7ull, 0xA24F77B4E02E2884ull,
     0x7E94646F74F9180Cull},
    {0xAFF8C635D325EC48ull, 0x2C2E0AA414038D0Bull, 0x4ED37F611A447467ull,
     0x39EC38E33B501489ull},
    {0x2A2BFDAD5F83F197ull, 0x013D3E6EBEF274CCull, 0xE1563C0477726155ull,
     0xF15A8A5DE932037Eull},
    {0xD5D1F91EC8126332ull, 0x10110B9BF9B1FF11ull, 0xA175AB26541C6032ull,
     0x87BADC5728701552ull},
    {0xC7B5A92CD8082884ull, 0xDDA62AB61B2EEEFBull, 0x8F9882ECFEAE732Full,
     0x6B38BD5CC01F4FFBull},
    {0xCF6EF275733D32F0ull, 0xA3F0822DA2BF7D8Bull, 0x304E7435F512406Aull,
     0x0B28E3EFEBB3172Dull},
    {0xE698F80701B2E9DBull, 0x66AE2A819A8A8828ull, 0x14EA9024C9B8F2C9ull,
     0xA7416170523EB5A4ull},
    {0x3A917E87E307EDB7ull, 0x17B4DEDAE34452C1ull, 0xF689F162E711CC70ull,
     0x29CE6BFE789CDD0Eull},
    {0x0EFF3AD8CB155D8Eull, 0x47CD9EAD4C0844A2ull, 0x46C8E40EE6FE21EBull,
     0xDEF3C25DF0340A51ull},
    {0x03FD86E62B82D04Dull, 0x32AB0D600717136Dull, 0x682B0E832B857A89ull,
     0x138CE3F1443739B1ull},
    {0x2F77C754C4D7F902ull, 0x1053E0A9D9ADBFEAull, 0x58E66368544AE70Aull,
     0xC48A829C72DD83CAull},
    {0xF900EB19E466A09Full, 0x31BE9E01A8C7D314ull, 0x3AFEC6B8CA08F471ull,
     0xB8C0EB0F87FFE7FBull},
    {0xDB277D8FBE3C8EFBull, 0x53CE6877E11AA57Bull, 0x719C94D20D9A7E7Dull,
     0xB345B56392453CC9ull},
    {0x37639C3BDBA4F2C9ull, 0x6095E7B336466DC8ull, 0x3A8049791E65B88Aull,
     0x82C988CDE5927CD5ull},
    {0x6B1FB1A714234AE4ull, 0x20562E255BA6467Eull, 0x3E2B892D40F3D675ull,
     0xF40CE3FBE41ED768ull},
    {0x8EE11CB1B287C92Aull, 0x8FC2AAEFF63D266Dull, 0x66643487E6EB9F03ull,
     0x578AA91DE8D56873ull},
    {0xF5B1F8266A3AEB67ull, 0x83B040BE4DEC1ADDull, 0x7FE1C8635B26FBAEull,
     0xF4A3A447DEFED79Full},
    {0x90D8E6FF6AC12475ull, 0x1A422A196EDAC1F2ull, 0x9E3765FE1F8EB002ull,
     0xC1BDD7C4C351CFBEull}};

void RunTests() {
  // TODO(janwas): detect number of cores.
  ThreadPool pool(4);

  TargetBits tested = ~0U;
  tested &= VerifyImplementations(kExpected64);
  tested &= VerifyImplementations(kExpected128);
  tested &= VerifyImplementations(kExpected256);
  // Any failure causes immediate exit, so apparently all succeeded.
  HH_TARGET_NAME::ForeachTarget(tested, [](const TargetBits target) {
    printf("%10s: OK\n", TargetName(target));
  });

  tested = ~0U;
  tested &= VerifyCat<HHResult64>(&pool);
  tested &= VerifyCat<HHResult128>(&pool);
  tested &= VerifyCat<HHResult256>(&pool);
  HH_TARGET_NAME::ForeachTarget(tested, [](const TargetBits target) {
    printf("%10sCat: OK\n", TargetName(target));
  });
}

#ifdef HH_GOOGLETEST
TEST(HighwayhashTest, OutputMatchesExpectations) { RunTests(); }
#endif

}  // namespace
}  // namespace highwayhash

#ifndef HH_GOOGLETEST
int main(int argc, char* argv[]) {
  highwayhash::RunTests();
  return 0;
}
#endif
