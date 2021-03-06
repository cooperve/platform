# Copyright (C) 2010 Google Inc. All rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#     * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above
# copyright notice, this list of conditions and the following disclaimer
# in the documentation and/or other materials provided with the
# distribution.
#     * Neither the name of Google Inc. nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

import os

from webkitpy.changelogs import ChangeLog
from webkitpy.grammar import pluralize
from webkitpy.steps.abstractstep import AbstractStep
from webkitpy.steps.options import Options
from webkitpy.webkit_logging import log, error

class UpdateChangeLogsWithReviewer(AbstractStep):
    @classmethod
    def options(cls):
        return [
            Options.reviewer,
        ]

    def _guess_reviewer_from_bug(self, bug_id):
        patches = self._tool.bugs.fetch_bug(bug_id).reviewed_patches()
        if len(patches) != 1:
            log("%s on bug %s, cannot infer reviewer." % (pluralize("reviewed patch", len(patches)), bug_id))
            return None
        patch = patches[0]
        log("Guessing \"%s\" as reviewer from attachment %s on bug %s." % (patch.reviewer().full_name, patch.id(), bug_id))
        return patch.reviewer().full_name

    def run(self, state):
        bug_id = state.get("bug_id")
        if not bug_id and state.get("patch"):
            bug_id = state.get("patch").bug_id()

        reviewer = self._options.reviewer
        if not reviewer:
            if not bug_id:
                log("No bug id provided and --reviewer= not provided.  Not updating ChangeLogs with reviewer.")
                return
            reviewer = self._guess_reviewer_from_bug(bug_id)

        if not reviewer:
            log("Failed to guess reviewer from bug %s and --reviewer= not provided.  Not updating ChangeLogs with reviewer." % bug_id)
            return

        os.chdir(self._tool.scm().checkout_root)
        for changelog_path in self._tool.scm().modified_changelogs():
            ChangeLog(changelog_path).set_reviewer(reviewer)
