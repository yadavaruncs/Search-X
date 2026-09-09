"""
Generates data/docs.txt: one synthetic short document per line.

Not part of the C++ engine -- this is just a one-time script to produce a
few thousand test documents so the index/ranking/benchmark have something
realistic-sized to run on. The word bank deliberately includes prefix
clusters (car/care/career/careful, data/database/dataset, search/season/
secure/security) so autocomplete demos have real suggestions to show.
"""
import random

random.seed(42)

WORD_BANK = [
    # deliberate prefix clusters for autocomplete demo
    "car", "care", "career", "careful", "careen", "caret", "cargo", "carbon", "carton",
    "data", "database", "dataset", "date", "day",
    "search", "season", "seat", "second", "secure", "security", "select",
    "code", "coder", "codebase", "coffee", "cold",
    "index", "indexed", "indexing", "inverted",
    "rank", "ranking", "ranked", "random",
    # general vocabulary to give documents realistic variety
    "algorithm", "system", "engine", "query", "document", "word", "token", "hash",
    "map", "tree", "heap", "cache", "memory", "thread", "process", "server",
    "client", "network", "file", "storage", "disk", "speed", "fast", "slow",
    "score", "relevance", "result", "user", "request", "response", "build",
    "test", "debug", "compile", "language", "program", "function", "class",
    "object", "pointer", "vector", "string", "number", "value", "key", "list",
    "graph", "node", "edge", "path", "queue", "stack", "sort", "search",
    "python", "java", "cpp", "compiler", "linker", "library", "module",
    "project", "college", "student", "interview", "resume", "skill",
]

TOPIC_SENTENCES = [
    "the {a} uses a {b} to speed up {c} operations",
    "every {a} improves {b} performance for {c} queries",
    "a good {a} makes {b} faster than a naive {c}",
    "students learn about {a} and {b} while building a {c}",
    "this {a} stores each {b} inside a {c} for quick lookup",
    "the {a} algorithm compares {b} against every {c} in the corpus",
    "engineers optimize the {a} to reduce {b} and improve {c}",
    "our {a} project demonstrates {b} concepts through a working {c}",
]

NUM_DOCS = 3000

def random_doc():
    template = random.choice(TOPIC_SENTENCES)
    a, b, c = random.sample(WORD_BANK, 3)
    sentence = template.format(a=a, b=b, c=c)
    extra = " ".join(random.choices(WORD_BANK, k=random.randint(3, 6)))
    return f"{sentence} {extra}"

def main():
    with open("data/docs.txt", "w", encoding="utf-8") as f:
        for _ in range(NUM_DOCS):
            f.write(random_doc() + "\n")
    print(f"wrote {NUM_DOCS} documents to data/docs.txt")

if __name__ == "__main__":
    main()
