"""Remove provably unused literal local initializers from derived native sources.

selcc emits stores for unused local arrays. No assignment, arithmetic, control
flow, table, coefficient, or synthesis statement is rewritten. The default trace
macro expands to nothing; trace calls are omitted only in this native build copy.
"""
import re


def unused_initializers(source):
    source=re.sub(r'^    MD_TRANSLATION_TRACE\([^\n]*\);\n','',source,flags=re.M)
    # Preserve offsets while excluding comments from identifier-use accounting.
    clean=re.sub(r'/\*.*?\*/|//[^\n]*',lambda m:re.sub(r'[^\n]',' ',m[0]),source,flags=re.S)
    functions=list(re.finditer(r'^(?:void|uint32_t|static uint32_t|static void) \w+\([^;\n]*\) \{',clean,re.M))
    edits=[]
    for match in functions:
        pos=match.end();depth=1
        while depth:
            if pos>=len(clean):raise ValueError('Unbalanced generated function')
            depth+=(clean[pos]=='{')-(clean[pos]=='}');pos+=1
        body=clean[match.end():pos-1]
        # Only integer literals and literal initializer lists are eligible.
        for declaration in re.finditer(r'^    (?:u?int32_t) (\w+)(?:\[\d+\])? = [0-9a-fxXuU{}, \-]+;\n',body,re.M):
            name=declaration[1]
            if len(re.findall(r'\b'+re.escape(name)+r'\b',body))==1:
                edits.append((match.end()+declaration.start(),match.end()+declaration.end()))
    for a,b in reversed(edits):source=source[:a]+source[b:]
    return source,len(edits)
