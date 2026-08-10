#!/usr/bin/env python3
import json, re, time
from dataclasses import dataclass
from typing import Dict, Any, Tuple

R_EQUILIBRIUM = 0.8158

class NewtonChainValidator:
    @staticmethod
    def validate(delta_e_kw: float, delta_r: float, max_r: float = R_EQUILIBRIUM) -> Tuple[str, bool]:
        is_valid = (delta_r <= max_r) and (delta_e_kw >= 0.0 or abs(delta_e_kw) <= 100.0)
        status = "VERIFIED" if is_valid else "REJECTED"
        sign = "+" if delta_e_kw >= 0 else ""
        return f"#NC[dE={sign}{delta_e_kw:.1f}kW,dR={delta_r:.4f},status={status}]", is_valid

@dataclass
class UNDClause:
    opcode: str
    domain: str
    attributes: Dict[str, Any]
    def to_und(self) -> str:
        attrs = ",".join(f"{k}:{v}" for k, v in self.attributes.items())
        return f"{self.opcode}{self.domain}{{{attrs}}}"

class UNDEncoder:
    def __init__(self, node_id: str = "NZ-01", r_limit: float = R_EQUILIBRIUM):
        self.node_id = node_id
        self.r_limit = r_limit
    def encode(self, telemetry: Dict[str, Any], delta_e_kw: float = 0.0, delta_r: float = 0.0) -> str:
        ts = telemetry.get("timestamp", int(time.time()))
        header = f"[{self.node_id}|{ts}|R={self.r_limit:.4f}]"
        clauses = [UNDClause(c["opcode"], c["domain"], c["attributes"]).to_und() for c in telemetry.get("clauses", [])]
        proof, _ = NewtonChainValidator.validate(delta_e_kw, delta_r, self.r_limit)
        return f"{header}::{';'.join(clauses)}::{proof}"

class UNDDecoder:
    FRAME_RE = re.compile(r"^\[(?P<node>[^|]+)\|(?P<ts>\d+)\|R=(?P<r>[\d\.]+)\]::(?P<clauses>.*?)::(?P<proof>#NC\[.*?\])$")
    PROOF_RE = re.compile(r"#NC\[dE=(?P<de>[^,]+),dR=(?P<dr>[\d\.]+),status=(?P<status>VERIFIED|REJECTED)\]")
    CLAUSE_RE = re.compile(r"([!?\~=%])(@[A-Z]+)\{([^}]+)\}")
    @classmethod
    def decode(cls, frame: str) -> Dict[str, Any]:
        m = cls.FRAME_RE.match(frame.strip())
        if not m: raise ValueError("Malformed UND frame")
        g = m.groupdict()
        clauses = []
        if g["clauses"]:
            for raw in g["clauses"].split(";"):
                cm = cls.CLAUSE_RE.match(raw)
                if cm:
                    op, dom, raw_attrs = cm.groups()
                    attrs = {k.strip(): v.strip() for item in raw_attrs.split(",") if ":" in item for k, v in [item.split(":", 1)]}
                    clauses.append({"opcode": op, "domain": dom, "attributes": attrs})
        pm = cls.PROOF_RE.match(g["proof"])
        if not pm: raise ValueError("Invalid proof")
        pg = pm.groupdict()
        return {"node_id": g["node"], "timestamp": int(g["ts"]), "r_equilibrium": float(g["r"]),
                "clauses": clauses, "newton_chain": {"delta_e": pg["de"], "delta_r": float(pg["dr"]),
                "status": pg["status"], "verified": pg["status"] == "VERIFIED"}}

if __name__ == "__main__":
    telem = {"timestamp": 1723284299, "clauses": [
        {"opcode": "?", "domain": "@THM", "attributes": {"T_sol": "330.15K"}},
        {"opcode": "?", "domain": "@PWR", "attributes": {"V_bat": "23.4V"}},
        {"opcode": "!", "domain": "@THM", "attributes": {"relay_1": "ON", "tgt": "bench_mass"}}]}
    enc = UNDEncoder()
    frame = enc.encode(telem, 1.2, 0.7410)
    print(frame)
    print(json.dumps(UNDDecoder.decode(frame), indent=2))
