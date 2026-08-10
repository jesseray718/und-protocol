#include "und_decoder.h"
#include <stdio.h>
bool parse_und_frame(const char* input, UndFrame* frame) {
    if (!input || !frame) return false;
    memset(frame, 0, sizeof(UndFrame));
    const char* d1 = strstr(input, "::"); if (!d1) return false;
    const char* d2 = strstr(d1 + 2, "::"); if (!d2) return false;
    if (input[0] != '[') return false;
    const char* pipe1 = strchr(input, '|');
    const char* pipe2 = pipe1 ? strchr(pipe1 + 1, '|') : NULL;
    if (!pipe1 || !pipe2 || pipe2 > d1) return false;
    size_t node_len = pipe1 - (input + 1);
    if (node_len >= UND_MAX_NODE_ID_LEN) node_len = UND_MAX_NODE_ID_LEN - 1;
    strncpy(frame->node_id, input + 1, node_len); frame->node_id[node_len] = '\0';
    frame->timestamp = (uint32_t)strtoul(pipe1 + 1, NULL, 10);
    const char* r_ptr = strstr(pipe2 + 1, "R=");
    if (r_ptr && r_ptr < d1) frame->r_equilibrium = (float)strtod(r_ptr + 2, NULL);
    const char* clause_ptr = d1 + 2;
    size_t clause_section_len = d2 - clause_ptr;
    if (clause_section_len > 0) {
        char clause_buf[256];
        if (clause_section_len >= sizeof(clause_buf)) clause_section_len = sizeof(clause_buf) - 1;
        strncpy(clause_buf, clause_ptr, clause_section_len); clause_buf[clause_section_len] = '\0';
        char* saveptr1; char* token = strtok_r(clause_buf, ";", &saveptr1);
        while (token && frame->clause_count < UND_MAX_CLAUSES) {
            UndClause* cl = &frame->clauses[frame->clause_count];
            char opcode = token[0];
            if (opcode == '!' || opcode == '?' || opcode == '\~' || opcode == '=' || opcode == '%') {
                cl->opcode = opcode;
                char* brace_open = strchr(token, '{'); char* brace_close = strchr(token, '}');
                if (brace_open && brace_close && brace_close > brace_open) {
                    size_t domain_len = brace_open - (token + 1);
                    if (domain_len >= UND_MAX_DOMAIN_LEN) domain_len = UND_MAX_DOMAIN_LEN - 1;
                    strncpy(cl->domain, token + 1, domain_len); cl->domain[domain_len] = '\0';
                    *brace_close = '\0'; char* attrs_str = brace_open + 1;
                    char* saveptr2; char* attr_pair = strtok_r(attrs_str, ",", &saveptr2);
                    while (attr_pair && cl->attr_count < UND_MAX_ATTRS_PER_CLAUSE) {
                        char* colon = strchr(attr_pair, ':');
                        if (colon) {
                            *colon = '\0';
                            strncpy(cl->attrs[cl->attr_count].key, attr_pair, UND_MAX_KEY_LEN - 1);
                            cl->attrs[cl->attr_count].key[UND_MAX_KEY_LEN - 1] = '\0';
                            strncpy(cl->attrs[cl->attr_count].val, colon + 1, UND_MAX_VAL_LEN - 1);
                            cl->attrs[cl->attr_count].val[UND_MAX_VAL_LEN - 1] = '\0';
                            cl->attr_count++;
                        }
                        attr_pair = strtok_r(NULL, ",", &saveptr2);
                    }
                }
                frame->clause_count++;
            }
            token = strtok_r(NULL, ";", &saveptr1);
        }
    }
    const char* proof_ptr = d2 + 2;
    const char* de_ptr = strstr(proof_ptr, "dE=");
    const char* dr_ptr = strstr(proof_ptr, "dR=");
    const char* st_ptr = strstr(proof_ptr, "status=");
    if (de_ptr) frame->proof.delta_e_kw = (float)strtod(de_ptr + 3, NULL);
    if (dr_ptr) frame->proof.delta_r = (float)strtod(dr_ptr + 3, NULL);
    bool status_verified = (st_ptr && strncmp(st_ptr + 7, "VERIFIED", 8) == 0);
    bool r_valid = (frame->proof.delta_r <= R_EQUILIBRIUM_LIMIT);
    frame->proof.is_verified = status_verified && r_valid;
    return frame->proof.is_verified;
}
