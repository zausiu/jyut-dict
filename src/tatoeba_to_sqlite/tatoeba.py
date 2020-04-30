import hanzidentifier
from hanziconv import HanziConv
from pypinyin import pinyin, Style

import sqlite3
import sys

source = {
    'name': '',
    'shortname': '',
    'version': '',
    'description': '',
    'legal': '',
    'link': '',
    'update_url': '',
    'other': ''
}

class ChineseSentence(object):
    def __init__(self, sentence_id=0, trad='', simp='', pin='', jyut='', lang=''):
        self.id = sentence_id
        self.traditional = trad
        self.simplified = simp
        self.pinyin = pin
        self.jyutping = jyut
        self.language = lang

class NonChineseSentence(object):
    def __init__(self, sentence_id=0, sentence='', lang=''):
        self.id=sentence_id
        self.sentence = sentence
        self.lang = lang

def write(chinese_sentences, nonchinese_sentences, links, db_name):
    db = sqlite3.connect(db_name)
    c = db.cursor()

    # Set version of database
    c.execute('PRAGMA user_version=2')

    # Delete old tables and indices
    c.execute('DROP TABLE IF EXISTS chinese_sentences')
    c.execute('DROP TABLE IF EXISTS sources')
    c.execute('DROP TABLE IF EXISTS nonchinese_sentences')
    c.execute('DROP TABLE IF EXISTS sentence_links')
    c.execute('DROP INDEX IF EXISTS fk_chinese_sentence_id_index')
    c.execute('DROP INDEX IF EXISTS fk_non_chinese_sentence_id_index')

    # Create new tables
    c.execute('''CREATE TABLE chinese_sentences(
                    chinese_sentence_id INTEGER PRIMARY KEY,
                    traditional TEXT,
                    simplified TEXT,
                    pinyin TEXT,
                    jyutping TEXT,
                    language TEXT,
                    fk_source_id INTEGER,
                    FOREIGN KEY(fk_source_id) REFERENCES sources(source_id),
                    UNIQUE(traditional, simplified, pinyin, jyutping) ON CONFLICT IGNORE
                )''')

    c.execute('''CREATE TABLE sources(
                    source_id INTEGER PRIMARY KEY,
                    sourcename TEXT UNIQUE ON CONFLICT ABORT,
                    sourceshortname TEXT,
                    version TEXT,
                    description TEXT,
                    legal TEXT,
                    link TEXT,
                    update_url TEXT,
                    other TEXT
                )''')

    c.execute('''CREATE TABLE nonchinese_sentences(
                    non_chinese_sentence_id INTEGER PRIMARY KEY,
                    sentence TEXT,
                    language TEXT,
                    fk_source_id INTEGER,
                    FOREIGN KEY(fk_source_id) REFERENCES sources(source_id),
                    UNIQUE(non_chinese_sentence_id, sentence) ON CONFLICT IGNORE
                )''')

    c.execute('''CREATE TABLE sentence_links(
                    fk_chinese_sentence_id INTEGER,
                    fk_non_chinese_sentence_id INTEGER,
                    FOREIGN KEY(fk_chinese_sentence_id) REFERENCES sentences(chinese_sentence_id),
                    FOREIGN KEY(fk_non_chinese_sentence_id) REFERENCES sentences(non_chinese_sentence_id)
                )''')

    # Add source to tables
    c.execute('INSERT INTO sources values(?,?,?,?,?,?,?,?,?)',
        (None, source['name'], source['shortname'], source['version'],
            source['description'], source['legal'], source['link'],
            source['update_url'], source['other']))

    # Add sentences to tables
    def chinese_sentence_to_tuple(sentence, source_id):
        return (sentence.id, sentence.traditional, sentence.simplified,
            sentence.pinyin, sentence.jyutping, sentence.language, source_id)

    def non_chinese_sentence_to_tuple(sentence, source_id):
        return (sentence.id, sentence.sentence, sentence.lang, source_id)

    for key in chinese_sentences:
        sentence = chinese_sentences[key]
        c.execute('INSERT INTO chinese_sentences values (?,?,?,?,?,?,?)',
            chinese_sentence_to_tuple(sentence, 1))

        c.execute('SELECT last_insert_rowid()')
        sentence_id = c.fetchone()[0]

    for key in nonchinese_sentences:
        sentence = nonchinese_sentences[key]
        c.execute('INSERT INTO nonchinese_sentences values (?,?,?,?)',
            non_chinese_sentence_to_tuple(sentence, 1))

    # Add links
    for source_id in links:
        target_id = links[source_id]
        c.execute('INSERT INTO sentence_links values (?,?)',
            (source_id, target_id))

    # Create indices
    # c.execute('CREATE INDEX fk_chinese_sentence_id_index ON sentence_links(fk_chinese_sentence_id)')
    # c.execute('CREATE INDEX fk_non_chinese_sentence_id_index ON sentence_links(fk_non_chinese_sentence_id)')

    db.commit()
    db.close()

# This function checks sentences.
# If it's a source sentence, we create a Chinese Sentence object
# Otherwise, we create a non-Chinese Sentence object
def parse_sentence_file(filename, source, target, sentences, nonchinese_sentences):
    with open(filename, 'r', encoding='utf8') as f:
        for index, line in enumerate(f):
            if (len(line) == 0 or line[0] == '#'):
                continue

            split = line.split()
            lang = split[1]
            sentence_start = line.index('\t', line.index('\t')+1) + 1
            sentence = line[sentence_start:]

            if lang == source:
                if (hanzidentifier.is_simplified(sentence)):
                    trad = HanziConv.toTraditional(sentence)
                    simp = sentence
                else:
                    trad = sentence
                    simp = HanziConv.toSimplified(sentence)
                pin = ' '.join(p[0] for p in pinyin(sentence,
                    style=Style.TONE3)).lower()
                sentence_id = split[0]
                sentence_row = ChineseSentence(sentence_id=sentence_id,
                    trad=trad, simp=simp, pin=pin, lang=lang)

                sentences[sentence_id] = sentence_row

            if lang == target:
                sentence_id = split[0]

                sentence = line[sentence_start:].strip()
                sentence_translation = NonChineseSentence(sentence_id=sentence_id,
                    lang=lang, sentence=sentence)
                nonchinese_sentences[sentence_id] = sentence_translation

def parse_links_file(filename, sentences, nonchinese_sentences,
    nonchinese_sentences_filtered, links):

    with open(filename, 'r', encoding='utf8') as f:
        for line in f:
            if (len(line) == 0 or line[0] == '#'):
                continue

            split = line.split()
            first_id = split[0]
            second_id = split[1]

            if first_id in sentences and second_id in nonchinese_sentences:
                links[first_id] = second_id
                nonchinese_sentences_filtered[second_id] = nonchinese_sentences[second_id]

if __name__ == '__main__':
    if len(sys.argv) != 14:
        print('Usage: python3 script.py <database filename> <Tatoeba sentences file> <Tatoeba links file> <source language> <target language> <source name> <source short name> <source version> <source description> <source legal> <source link> <source update url> <source other>')
        print('e.g. python3 script.py dict.db sentences.csv links.csv yue eng Tatoeba TTB 2018-07-09 "Tatoeba is a collection of sentences." "These files are released under CC BY 2.0 FR." "https://tatoeba.org/eng/downloads" "" ""')
        sys.exit(1)

    chinese_sentences = {}     # Use this to store all the source sentences
    nonchinese_sentences = {}  # Use this to store all the target sentences
    nonchinese_sentences_filtered = {} # Store only target sentences that match a source sentence
    links = {}                 # Use this to store all the links between sentences
    source['name'] = sys.argv[6]
    source['shortname'] = sys.argv[7]
    source['version'] = sys.argv[8]
    source['description'] = sys.argv[9]
    source['legal'] = sys.argv[10]
    source['link'] = sys.argv[11]
    source['update_url'] = sys.argv[12]
    source['other'] = sys.argv[13]
    parse_sentence_file(sys.argv[2], sys.argv[4], sys.argv[5], chinese_sentences, nonchinese_sentences)
    parse_links_file(sys.argv[3], chinese_sentences, nonchinese_sentences, nonchinese_sentences_filtered, links)
    write(chinese_sentences, nonchinese_sentences_filtered, links, sys.argv[1])
