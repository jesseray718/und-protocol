#ifndef UND_DECODER_H
#define UND_DECODER_H
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
#define R_EQUILIBRIUM_LIMIT 0.8158f
#define UND_MAX_NODE_ID_LEN 16
#define UND_MAX_CLAUSES 6
#define UND_MAX_ATTRS_PER_CLAUSE 4
#define UND_MAX_KEY_LEN 12
#define UND_MAX_VAL_LEN 16
#define UND_MAX_DOMAIN_LEN 8
typedef struct { char key[UND_MAX_KEY_LEN]; char val[UND_MAX_VAL_LEN]; } UndAttr;
typedef struct { char opcode; char domain[UND_MAX_DOMAIN_LEN]; UndAttr attrs[UND_MAX_ATTRS_PER_CLAUSE]; uint8_t attr_count; } UndClause;
typedef struct { float delta_e_kw; float delta_r; bool is_verified; } NewtonProof;
typedef struct { char node_id[UND_MAX_NODE_ID_LEN]; uint32_t timestamp; float r_equilibrium; UndClause clauses[UND_MAX_CLAUSES]; uint8_t clause_count; NewtonProof proof; } UndFrame;
bool parse_und_frame(const char* input, UndFrame* frame);
#ifdef __cplusplus
}
#endif
#endif
