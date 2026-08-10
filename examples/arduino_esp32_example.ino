#include <Arduino.h>
#include "und_decoder.h"
const char* raw_und_frame = "[NZ-01|1723284299|R=0.8158]::?@THM{T_sol:330.15K};!@THM{relay_1:ON,tgt:bench_mass}::#NC[dE=+1.2kW,dR=0.7410,status=VERIFIED]";
void setup() {
    Serial.begin(115200);
    while (!Serial) delay(10);
    Serial.println("\n--- UND PROTOCOL DECODER (ESP32) ---");
    UndFrame frame;
    if (parse_und_frame(raw_und_frame, &frame)) {
        Serial.println("[DECODE SUCCESS]");
        Serial.printf("Node: %s  R=%.4f  Clauses=%d\n", frame.node_id, frame.r_equilibrium, frame.clause_count);
        for (uint8_t i = 0; i < frame.clause_count; i++) {
            Serial.printf("  %c%s\n", frame.clauses[i].opcode, frame.clauses[i].domain);
            for (uint8_t j = 0; j < frame.clauses[i].attr_count; j++)
                Serial.printf("    %s=%s\n", frame.clauses[i].attrs[j].key, frame.clauses[i].attrs[j].val);
        }
    } else {
        Serial.println("[REJECTED]");
    }
}
void loop() { delay(5000); }
